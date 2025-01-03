#include "UI.hpp"
#include "ui/ComponentsEdit.hpp"

#include <engine/components/Components.hpp>

#include <engine/PerspectiveCamera.hpp>
#include <engine/NewResourceManager.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <opengl/FrameBuffer.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <ImGuizmo.h>

#include <flecs.h>

namespace ui {

template<typename T>
inline void visitIfExists(auto &&v, flecs::entity &e) {
    if (auto *component = e.get_mut<T>())
        v(*component);
};

inline void visitComponents(auto &&v, flecs::entity &e) {
    visitIfExists<engine::components::Transform>(v, e);
    visitIfExists<engine::components::Material>(v, e);
    visitIfExists<engine::components::StaticMesh>(v, e);
    visitIfExists<engine::components::AnimatedMesh>(v, e);
    visitIfExists<engine::components::PointLight>(v, e);
    visitIfExists<engine::components::DirectionalLight>(v, e);
    visitIfExists<engine::components::Skybox>(v, e);
}

void draw(EditorContext &context)
{
    ImGui::DockSpaceOverViewport();

    if (ImGui::Begin("Main window"))
    {
        glm::vec3 position = context.camera.getPosition();
        glm::vec3 rotation = context.camera.getRotation();
        float fov = context.camera.getFov();

        ImGui::DragFloat3("Position", glm::value_ptr(position), 0.25f);
        ImGui::DragFloat3("Rotation", glm::value_ptr(rotation), 3.f, -180.f, 180.f);
        ImGui::SliderFloat("Fov", &fov, 15.f, 120.f);

        context.camera.setPosition(position);
        context.camera.setRotation(rotation);
        context.camera.setFov(fov);
    }
    ImGui::End();


    if (ImGui::Begin("Scene editor"))
    {
        context.world.each([&context](flecs::entity e, engine::tags::SceneEntityTag) {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
            bool hasChildren = false;
            e.children([&hasChildren, e](flecs::entity ce) {
                INFO("{} is child of {}", ce.name().c_str(), e.name().c_str());
                hasChildren = true;
            });
            if (!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf;
            if (ImGui::TreeNodeEx(std::format("{} ({})", e.name().c_str(), e.id()).c_str(), flags))
            {
                ImGui::TreePop();
            }
            if (ImGui::IsItemClicked())
            {
                context.selectedEntity = e;
            }
        });
    }
    ImGui::End();

    if (ImGui::Begin("Selected entity"))
    {
        visitComponents([&context](auto &component) {
            using ComponentType = decltype(component);
            flecs::component<ComponentType> componentEntity
                = context.world.component<ComponentType>();
            if (ImGui::CollapsingHeader(
                componentEntity.name().c_str(),
                ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::PushID(componentEntity.name().c_str());
                ui::componentEdit(context, component);
                ImGui::PopID();
            }
        }, context.selectedEntity);
    }
    ImGui::End();

    if (ImGui::Begin("Scene"))
    {
        if (ImGui::BeginChild("Scene Render"))
        {
            ImVec2 size = ImGui::GetContentRegionAvail();
            // resize when size changes
            if (size.x != (float)context.renderTarget.getWidth()
                || size.y != (float)context.renderTarget.getHeight())
            {
                context.resizeCallback(size.x, size.y);
            }

            ImGui::Image(
                (ImTextureID)(intptr_t)(context.renderTarget.getColorAttachments()[0]->getId()),
                size,
                ImVec2(0, 1),
                ImVec2(1, 0));

            {
                ImGuizmo::SetOrthographic(false);
                ImGuizmo::SetDrawlist();
                float windowWidth = ImGui::GetWindowWidth();
                float windowHeight = ImGui::GetWindowHeight();
                ImGuizmo::SetRect(
                    ImGui::GetWindowPos().x,
                    ImGui::GetWindowPos().y,
                    windowWidth,
                    windowHeight);

                const glm::mat4 &cameraProj = context.camera.getProjectionMatrix();
                const glm::mat4 &cameraView = context.camera.getViewMatrix();

                ASSERT(context.selectedEntity.has<engine::components::Transform>());
                glm::mat4 selectedEntityTransform
                    = *context.selectedEntity.get<engine::components::Transform>();

                ImGuizmo::Manipulate(
                    glm::value_ptr(cameraView),
                    glm::value_ptr(cameraProj),
                    ImGuizmo::OPERATION::ROTATE,
                    ImGuizmo::LOCAL,
                    glm::value_ptr(selectedEntityTransform));

                if (ImGuizmo::IsUsing())
                {
                    // Already ensured that the component exists above
                    engine::components::Transform &transform
                        = *context.selectedEntity.get_mut<engine::components::Transform>();
                    ImGuizmo::DecomposeMatrixToComponents(
                        glm::value_ptr(selectedEntityTransform),
                        glm::value_ptr(transform.position),
                        glm::value_ptr(transform.rotation),
                        glm::value_ptr(transform.scale));
                }

            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

} // namespace ui
