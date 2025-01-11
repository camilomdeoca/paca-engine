#include "UI.hpp"
#include "engine/IdTypes.hpp"
#include "engine/Loader.hpp"
#include "nfd.h"
#include "ui/ComponentsEdit.hpp"

#include <algorithm>
#include <cstring>
#include <engine/Components.hpp>

#include <engine/PerspectiveCamera.hpp>
#include <engine/NewResourceManager.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <opengl/FrameBuffer.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <ImGuizmo.h>
#include <implot.h>

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

void drawSelectedEntity(EditorContext &context)
{
    if (ImGui::Begin("Selected entity"))
    {
        if (context.selectedEntity.is_valid())
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
    }
    ImGui::End();
}

void drawSceneTree(EditorContext &context)
{
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
}

void drawCameraParameters(EditorContext &context)
{
    if (ImGui::Begin("Camera parameters"))
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
}

void drawStats(EditorContext &context)
{
    if (ImGui::Begin("Stats"))
    {
        constexpr float maxHistoryLength = 10.0f;
        constexpr size_t maxSize = maxHistoryLength * 60.0f;

        static float frametimeHistoryLength = 3.0f; // in seconds
        static size_t newestFrametimeIndex = 0;
        static std::vector<ImVec2> pastFrametimesPoints = []() {
            std::vector<ImVec2> res;
            res.reserve(maxSize);
            return res;
        }();

        ImGui::SliderFloat("Frametime history length", &frametimeHistoryLength, 0.1f,
                maxHistoryLength);


        float currentTime = context.time / 1000.0f; // from ms to seconds

        if (pastFrametimesPoints.size() < maxSize)
        {
            pastFrametimesPoints.emplace_back(currentTime, context.timeDelta);
        }
        else
        {
            pastFrametimesPoints[newestFrametimeIndex] = ImVec2(currentTime, context.timeDelta);
            newestFrametimeIndex = (newestFrametimeIndex + 1) % maxSize;
        }


        if (ImPlot::BeginPlot("##FrametimePlot", ImVec2(-1, 150)))
        {
            ImPlot::SetupAxes(nullptr, "Frametime (ms)", 0, 0);
            ImPlot::SetupAxisLimits(ImAxis_X1, currentTime - frametimeHistoryLength, currentTime,
                    ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0f, 40.0f);
            ImPlot::PlotLine("Frametime", &pastFrametimesPoints[0].x, &pastFrametimesPoints[0].y,
                    pastFrametimesPoints.size(), ImPlotLineFlags_None, (int)newestFrametimeIndex,
                    sizeof(ImVec2));
            ImPlot::EndPlot();
        }
    }
    ImGui::End();
}

bool openFile(char *outPath, size_t pathBufferSize, const std::vector<nfdu8filteritem_t> &filters)
{
    nfdresult_t result = NFD_Init();
    if (result != NFD_OKAY)
    {
        ERROR("NFD initializing error:");
        ERROR("{}", NFD_GetError());
        return false;
    }
    nfdu8char_t *nfdOutPath;
    nfdopendialogu8args_t args = {0};
    args.filterList = filters.data();
    args.filterCount = filters.size();
    result = NFD_OpenDialogU8_With(&nfdOutPath, &args);
    if (result == NFD_OKAY)
    {
        outPath[0] = '\0';
        strncat(outPath, nfdOutPath, pathBufferSize - 1);
        NFD_FreePathU8(nfdOutPath);
    }
    else if (result == NFD_CANCEL)
    {
    }
    else
    {
        ERROR("Error importing static mesh file: {}", outPath);
        ERROR("{}", NFD_GetError());
    }
    NFD_Quit();
    return result == NFD_OKAY;
}

void drawAssetManager(EditorContext &context)
{
    if (ImGui::Begin("Assets"))
    {
        bool open = ImGui::TreeNodeEx("Static meshes");
        if (ImGui::BeginPopupContextItem("Operations"))
        {
            if (ImGui::Selectable("Import static mesh"))
            {
                char meshPath[256]{};
                if (openFile(meshPath, sizeof(meshPath), {{ "GLTF Model", "gltf,glb" }}))
                {
                    // We want to find the lowest free id, that is the same as finding the
                    // lowest id in the sorted array that doesn't match its index + 1 (we start
                    // ids at 1 because 0 is the null id)
                    std::sort(
                        context.assetPack.staticMeshes.begin(),
                        context.assetPack.staticMeshes.end(),
                        [](
                            const paca::fileformats::StaticMeshRef &m1,
                            const paca::fileformats::StaticMeshRef &m2) -> bool
                        {
                            return m1.id < m2.id;
                        });
                    paca::fileformats::StaticMeshId lowestFreeId = context.assetPack.staticMeshes.size() + 1;
                    for (size_t i = 0; i < context.assetPack.staticMeshes.size(); i++)
                    {
                        const paca::fileformats::StaticMeshRef &mesh = context.assetPack.staticMeshes[i];
                        if (mesh.id != i + 1)
                        {
                            lowestFreeId = i + 1;
                            break;
                        }
                    }

                    context.assetPack.staticMeshes.emplace_back(meshPath, lowestFreeId, meshPath);

                    auto mesh = engine::loaders::load<paca::fileformats::StaticMesh>(meshPath);
                    mesh->name = meshPath;
                    mesh->id = lowestFreeId;
                    context.resourceManager.add(*mesh);

                    INFO("Imported static mesh file: {}", meshPath);
                }
            }
            ImGui::EndPopup();
        }

        if (open)
        {
            paca::fileformats::StaticMeshId toDeleteId = 0;
            for (auto &mesh : context.assetPack.staticMeshes)
            {
                ImGui::PushID(mesh.id);
                ImGui::BeginGroup();
                if (ImGui::TreeNodeEx(&mesh.id, 0, "%s", mesh.name.c_str()))
                {
                    char buf[256] = "";
                    memcpy(buf, mesh.name.c_str(), mesh.name.size() + 1); // to copy also '\0'
                    ImGui::InputText("Name", buf, 256);
                    mesh.name = buf;

                    ImGui::LabelText("Id", "%u", mesh.id);

                    ImGui::TreePop();
                }
                ImGui::EndGroup();
                if (ImGui::BeginPopupContextItem("Operations"))
                {
                    if (ImGui::Selectable("Delete"))
                    {
                        toDeleteId = mesh.id;
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopID();
            }

            if (toDeleteId != 0)
            {
                auto it = std::find_if(
                    context.assetPack.staticMeshes.begin(),
                    context.assetPack.staticMeshes.end(),
                    [toDeleteId](const paca::fileformats::StaticMeshRef &mesh) {
                        return mesh.id == toDeleteId;
                    });
                std::swap(*it, context.assetPack.staticMeshes.back());
                context.assetPack.staticMeshes.pop_back();
                context.resourceManager.remove(StaticMeshId(toDeleteId));
            }

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void drawSceneView(EditorContext &context)
{
    if (ImGui::Begin("Scene"))
    {
        if (ImGui::BeginChild("Scene Render"))
        {
            if(ImGui::IsWindowFocused())
            {
                constexpr float cameraSpeed = 0.005f; // units per ms

                glm::vec3 position = context.camera.getPosition();
                glm::vec3 front = context.camera.getDirection();
                //front.y = 0;
                //front = glm::normalize(front);
                glm::vec3 right = glm::normalize(glm::cross(front, context.camera.getUp()));

                if (ImGui::IsKeyDown(ImGuiKey_A)) {
                    position -= right * cameraSpeed * context.timeDelta;
                }
                if (ImGui::IsKeyDown(ImGuiKey_D)) {
                    position += right * cameraSpeed * context.timeDelta;
                }
                if (ImGui::IsKeyDown(ImGuiKey_W)) {
                    position += front * cameraSpeed * context.timeDelta;
                }
                if (ImGui::IsKeyDown(ImGuiKey_S)) {
                    position -= front * cameraSpeed * context.timeDelta;
                }
                if (ImGui::IsKeyDown(ImGuiKey_Space)) {
                    position.y += cameraSpeed * context.timeDelta;
                }
                if (ImGui::IsKeyDown(ImGuiKey_ModShift)) {
                    position.y -= cameraSpeed * context.timeDelta;
                }

                if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
                {
                    ImGuiIO& io = ImGui::GetIO();
                    const float sensitivity = 0.25f;
                    glm::vec3 rotation = context.camera.getRotation();
                    float fov = context.camera.getFov();

                    rotation.x -= io.MouseDelta.y * sensitivity * (fov/90.0f);
                    rotation.y += io.MouseDelta.x * sensitivity * (fov/90.0f);

                    rotation.x = std::clamp(rotation.x, -89.0f, 89.0f);

                    context.camera.setRotation(rotation);
                }

                context.camera.setPosition(position);
            }

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

            if (context.selectedEntity.is_valid())
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
                    = context.selectedEntity.get<engine::components::Transform>()->getTransform();

                static ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
                if (ImGui::IsWindowHovered())
                {
                    if (ImGui::IsKeyDown(ImGuiKey_T)) operation = ImGuizmo::TRANSLATE;
                    if (ImGui::IsKeyDown(ImGuiKey_R)) operation = ImGuizmo::ROTATE;
                    if (ImGui::IsKeyDown(ImGuiKey_G)) operation = ImGuizmo::SCALE;
                }

                ImGuizmo::Manipulate(
                    glm::value_ptr(cameraView),
                    glm::value_ptr(cameraProj),
                    operation,
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

void draw(EditorContext &context)
{
    ImGui::DockSpaceOverViewport();

    drawStats(context);
    drawSceneTree(context);
    drawSceneView(context);
    drawAssetManager(context);
    drawSelectedEntity(context);
    drawCameraParameters(context);
}

} // namespace ui
