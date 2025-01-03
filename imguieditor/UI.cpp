#include "UI.hpp"
#include "ui/ComponentsEdit.hpp"

#include <engine/components/Components.hpp>

#include <engine/PerspectiveCamera.hpp>
#include <engine/NewResourceManager.hpp>
#include <opengl/FrameBuffer.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include <flecs.h>
#include <type_traits>

namespace ui {

inline void visitComponents(auto &&v, flecs::entity e) {
#define VISIT_IF_EXISTS(type) {          \
    if (auto *component = e.get_mut<type>()) \
        v(*component);                   \
}
    VISIT_IF_EXISTS(engine::components::Transform);
    VISIT_IF_EXISTS(engine::components::Material);
    VISIT_IF_EXISTS(engine::components::StaticMesh);
    VISIT_IF_EXISTS(engine::components::AnimatedMesh);
    VISIT_IF_EXISTS(engine::components::PointLight);
    VISIT_IF_EXISTS(engine::components::DirectionalLight);
    VISIT_IF_EXISTS(engine::components::Skybox);
#undef VISIT_IF_EXISTS
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
                ui::componentEdit(context, component);
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
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

} // namespace ui
