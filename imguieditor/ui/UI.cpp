#include "UI.hpp"
#include "CustomComponents.hpp"

#include <engine/IdTypes.hpp>
#include <format>
#include <nfd.h>

#include <engine/Components.hpp>
#include <engine/PerspectiveCamera.hpp>
#include <engine/AssetManager.hpp>
#include <opengl/FrameBuffer.hpp>
#include <opengl/gl.hpp>

#include <glm/ext/quaternion_common.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <ImGuizmo.h>
#include <implot.h>

#include <flecs.h>

#include <algorithm>
#include <cstring>
#include <tuple>
#include <unordered_set>
#include <utility>

constexpr const char * STATIC_MESH_ID_DRAG_DROP_PAYLOAD_TYPE = "StaticMeshId";
constexpr const char * MATERIAL_ID_DRAG_DROP_PAYLOAD_TYPE = "MaterialId";


UI::UI(const Context &context)
    : m_assetManager(context.assetManager),
      m_assetMetadataManager(context.assetMetadataManager),
      m_renderer(context.renderer),
      m_world(context.world),
      m_time(0.0f),
      m_timeDelta(0.0f)
{}

void UI::draw()
{
    ImGui::DockSpaceOverViewport();

    drawStats();
    drawSceneTree();
    drawSceneView();
    drawAssetManager();
    drawSelectedEntity();
    drawCameraParameters();
    drawShadowMapsDebug();
    drawCameraEntityViews();
}

void UI::drawStats()
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


        float currentTime = m_time / 1000.0f; // from ms to seconds

        if (pastFrametimesPoints.size() < maxSize)
        {
            pastFrametimesPoints.emplace_back(currentTime, m_timeDelta);
        }
        else
        {
            pastFrametimesPoints[newestFrametimeIndex] = ImVec2(currentTime, m_timeDelta);
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

void UI::drawSceneTree()
{
    if (ImGui::Begin("Scene editor"))
    {
        m_world.each([this](flecs::entity e, engine::tags::SceneEntityTag) {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
            bool hasChildren = false;
            e.children([&hasChildren, e](flecs::entity ce) {
                INFO("{} is child of {}", ce.name().c_str(), e.name().c_str());
                hasChildren = true;
            });
            if (!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
            if (ImGui::TreeNodeEx(std::format("{} ({})", e.name().c_str(), e.id()).c_str(), flags))
            {
                ImGui::TreePop();
            }
            if (ImGui::IsItemClicked())
            {
                m_selectedEntity = e;
            }

            if (e.has<engine::components::Camera>() && ImGui::BeginPopupContextItem("Operations"))
            {
                if (ImGui::Selectable("Preview"))
                {
                    CameraEntityView &cameraEntityView = m_cameraViews.emplace_back();
                    cameraEntityView.entity = e;
                }

                ImGui::EndPopup();
            }
        });
    }
    ImGui::End();
}

void UI::drawSceneView()
{
    if (ImGui::Begin("Scene"))
    {
        if (ImGui::BeginChild("Scene Render"))
        {
            if(ImGui::IsWindowFocused())
            {
                constexpr float cameraSpeed = 0.005f; // units per ms

                glm::vec3 front = m_cameraTransform.getDirection();
                glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
                //front.y = 0;
                //front = glm::normalize(front);
                glm::vec3 right = glm::normalize(glm::cross(front, up));

                if (ImGui::IsKeyDown(ImGuiKey_A)) {
                    m_cameraTransform.position -= right * cameraSpeed * m_timeDelta;
                }
                if (ImGui::IsKeyDown(ImGuiKey_D)) {
                    m_cameraTransform.position += right * cameraSpeed * m_timeDelta;
                }
                if (ImGui::IsKeyDown(ImGuiKey_W)) {
                    m_cameraTransform.position += front * cameraSpeed * m_timeDelta;
                }
                if (ImGui::IsKeyDown(ImGuiKey_S)) {
                    m_cameraTransform.position -= front * cameraSpeed * m_timeDelta;
                }
                if (ImGui::IsKeyDown(ImGuiKey_Space)) {
                    m_cameraTransform.position.y += cameraSpeed * m_timeDelta;
                }
                if (ImGui::IsKeyDown(ImGuiKey_ModShift)) {
                    m_cameraTransform.position.y -= cameraSpeed * m_timeDelta;
                }

                if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
                {
                    ImGuiIO& io = ImGui::GetIO();
                    const float sensitivity = 0.25f;

                    m_cameraTransform.rotation.x -= io.MouseDelta.y * sensitivity * (m_camera.fov/90.0f);
                    m_cameraTransform.rotation.y += io.MouseDelta.x * sensitivity * (m_camera.fov/90.0f);

                    m_cameraTransform.rotation.x = std::clamp(m_cameraTransform.rotation.x, -89.0f, 89.0f);
                }
            }

            ImVec2 size = ImGui::GetContentRegionAvail();
            // resize when size changes
            if (size.x != (float)m_renderTarget.getWidth()
                || size.y != (float)m_renderTarget.getHeight())
            {
                INFO("prev {} {} next {} {}", m_renderTarget.getWidth(), m_renderTarget.getHeight(), size.x, size.y);
                m_renderTarget.shutdown();
                Texture depthAttachment(Texture::Specification{
                    .width = static_cast<uint32_t>(size.x),
                    .height = static_cast<uint32_t>(size.y),
                    .format = Texture::Format::depth24,
                });

                std::vector<Texture> colorTextures;
                colorTextures.emplace_back(Texture::Specification{
                    .width = static_cast<uint32_t>(size.x),
                    .height = static_cast<uint32_t>(size.y),
                    .format = Texture::Format::RGB8,
                    .linearMinification = false,
                    .linearMagnification = false,
                    .interpolateBetweenMipmapLevels = false,
                    .tile = false,
                });
                m_renderTarget.init({
                    .width = static_cast<uint32_t>(size.x),
                    .height = static_cast<uint32_t>(size.y),
                    .depthTextureAttachment = std::move(depthAttachment),
                    .colorTextureAttachments = std::move(colorTextures),
                });
                m_camera.aspect = size.x / size.y;
            }

            m_renderTarget.bind();
            GL::clear();
            m_renderer.renderWorld(m_cameraTransform, m_camera, m_world, m_assetManager, m_renderTarget);

            ImGui::Image(
                (ImTextureID)(intptr_t)(m_renderTarget.getColorAttachments()[0].getId()),
                size,
                ImVec2(0, 1),
                ImVec2(1, 0));

            if (m_selectedEntity.is_valid())
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

                const glm::mat4 cameraProj = m_camera.getProjection();
                const glm::mat4 cameraView = m_cameraTransform.getView();

                ASSERT(m_selectedEntity.has<engine::components::Transform>());
                glm::mat4 selectedEntityTransform
                    = m_selectedEntity.get<engine::components::Transform>()->getTransform();

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
                        = *m_selectedEntity.get_mut<engine::components::Transform>();
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

void UI::addStaticMesh(const char *meshPath)
{
    // We want to find the lowest free id, that is the same as finding the
    // lowest id in the sorted array that doesn't match its index + 1 (we start
    // ids at 1 because 0 is the null id)
    std::vector<paca::fileformats::StaticMeshId> ids;
    ids.reserve(m_assetMetadataManager.staticMeshes().size());
    for (const auto &[id, metadata] : m_assetMetadataManager.staticMeshes())
    {
        ids.emplace_back(std::to_underlying(id));
    }

    std::ranges::sort(
        ids,
        [](
            const paca::fileformats::StaticMeshId &id1,
            const paca::fileformats::StaticMeshId &id2) -> bool
        {
            return id1 < id2;
        });

    paca::fileformats::StaticMeshId lowestFreeId = ids.size() + 1;
    for (size_t i = 0; i < ids.size(); i++)
    {
        if (ids[i] != i + 1)
        {
            lowestFreeId = i + 1;
            break;
        }
    }

    paca::fileformats::StaticMeshRef mesh = {
        .name = meshPath,
        .id = lowestFreeId,
        .path = meshPath,
    };
    m_assetMetadataManager.add(mesh);

    INFO("Imported static mesh file: {}", meshPath);
}

void UI::drawAssetManager()
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_None;
    flags |= ImGuiWindowFlags_MenuBar;

    if (ImGui::Begin("Assets", nullptr, flags))
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::BeginMenu("Import assets"))
                {
                    if (ImGui::MenuItem("Import static mesh"))
                    {
                        char meshPath[256] = {0};
                        if (openFile(meshPath, sizeof(meshPath), {{ "GLTF Model", "gltf,glb" }}))
                        {
                            addStaticMesh(meshPath);
                        }
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        if (ImGui::BeginTabBar("Asset types tab bar"))
        {
            if (ImGui::BeginTabItem("StaticMeshes"))
            {
                drawStaticMeshBrowser();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Materials"))
            {
                drawMaterialBrowser();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

void UI::drawStaticMeshBrowser()
{
    char searchQuery[128] = {0};
    ImGui::InputTextWithHint("Search", "Name...", searchQuery, sizeof(searchQuery));

    struct {
        StaticMeshId from = StaticMeshId::null;
        StaticMeshId to   = StaticMeshId::null;
    } toChangeId;

    static std::unordered_set<StaticMeshId> selectedStaticMeshes;
    for (auto &[id, meshMetadata] : m_assetMetadataManager.staticMeshes())
    {
        if (searchQuery[0] != '\0' && !meshMetadata.name.contains(searchQuery))
            continue;

        ImGui::PushID(std::to_underlying(id));

        ImGui::SetNextItemAllowOverlap();
        if (ImGui::Selectable("", selectedStaticMeshes.contains(id), 0, ImVec2(0, 96)))
        {
            if (!ImGui::GetIO().KeyCtrl) selectedStaticMeshes.clear();
            selectedStaticMeshes.insert(id);
        }

        if (ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload(STATIC_MESH_ID_DRAG_DROP_PAYLOAD_TYPE, &id, sizeof(id));

            ImGui::TextUnformatted(meshMetadata.name.c_str());
            ImGui::Image(
                (ImTextureID)(intptr_t)(meshMetadata.preview.getId()),
                ImVec2(96, 96),
                ImVec2(0, 1),
                ImVec2(1, 0));

            ImGui::EndDragDropSource();
        }

        ImGui::SameLine(0.0f, 0.0f);
        ImGui::Image(
            (ImTextureID)(intptr_t)(meshMetadata.preview.getId()),
            ImVec2(96, 96),
            ImVec2(0, 1),
            ImVec2(1, 0));
        ImGui::SameLine();
        ImGui::BeginGroup();
        {
            char name[128] = "";
            char path[128] = "";
            strncpy(name, meshMetadata.name.c_str(), sizeof(name));
            strncpy(path, meshMetadata.path.c_str(), sizeof(path));

            ImGui::InputText("Name", name, sizeof(name));
            ImGui::InputText("Path", path, sizeof(path));
            auto underlyingId = std::to_underlying(id);
            if (ImMe::InputUInt("Id", &underlyingId)
                && StaticMeshId(underlyingId) != StaticMeshId::null)
            {
                toChangeId.from = id;
                toChangeId.to = StaticMeshId(underlyingId);
            }
        }
        ImGui::EndGroup();

        if (ImGui::BeginPopupContextItem("Operations"))
        {
            if (ImGui::Selectable("Edit"))
            {
            }

            if (ImGui::Selectable("Remove"))
            {
                toChangeId.from = id;
                toChangeId.to = StaticMeshId::null; // just in case
            }
            ImGui::EndPopup();
        }

        ImGui::PopID();
    }

    if (toChangeId.from != StaticMeshId::null)
    {
        if (toChangeId.to != StaticMeshId::null)
        {
            m_assetMetadataManager.move(toChangeId.from, toChangeId.to);
        }
        else
        {
            m_assetMetadataManager.remove(toChangeId.from);
        }
    }
}

void UI::drawMaterialBrowser()
{
    char searchQuery[128] = {0};
    ImGui::InputTextWithHint("Search", "Name...", searchQuery, sizeof(searchQuery));

    struct {
        MaterialId from = MaterialId::null;
        MaterialId to   = MaterialId::null;
    } toChangeId;

    static std::unordered_set<MaterialId> selectedMaterials;
    for (auto &[id, materialMetadata] : m_assetMetadataManager.materials())
    {
        if (searchQuery[0] != '\0' && !materialMetadata.name.contains(searchQuery))
            continue;

        ImGui::PushID(std::to_underlying(id));

        ImGui::SetNextItemAllowOverlap();
        if (ImGui::Selectable("", selectedMaterials.contains(id), 0, ImVec2(0, 96)))
        {
            if (!ImGui::GetIO().KeyCtrl) selectedMaterials.clear();
            selectedMaterials.insert(id);
        }

        if (ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload(MATERIAL_ID_DRAG_DROP_PAYLOAD_TYPE, &id, sizeof(id));

            ImGui::TextUnformatted(materialMetadata.name.c_str());
            ImGui::Image(
                (ImTextureID)(intptr_t)(materialMetadata.preview.getId()),
                ImVec2(96, 96),
                ImVec2(0, 1),
                ImVec2(1, 0));

            ImGui::EndDragDropSource();
        }

        ImGui::SameLine(0.0f, 0.0f);
        ImGui::Image(
            (ImTextureID)(intptr_t)(materialMetadata.preview.getId()),
            ImVec2(96, 96),
            ImVec2(0, 1),
            ImVec2(1, 0));
        ImGui::SameLine();
        ImGui::BeginGroup();
        {
            char name[128] = "";
            strncpy(name, materialMetadata.name.c_str(), sizeof(name));

            ImGui::InputText("Name", name, sizeof(name));
            auto underlyingId = std::to_underlying(id);
            if (ImMe::InputUInt("Id", &underlyingId)
                && MaterialId(underlyingId) != MaterialId::null)
            {
                toChangeId.from = id;
                toChangeId.to = MaterialId(underlyingId);
            }
        }
        ImGui::EndGroup();

        if (ImGui::BeginPopupContextItem("Operations"))
        {
            if (ImGui::Selectable("Edit"))
            {
            }

            if (ImGui::Selectable("Remove"))
            {
                toChangeId.from = id;
                toChangeId.to = MaterialId::null; // just in case
            }
            ImGui::EndPopup();
        }

        ImGui::PopID();
    }

    if (toChangeId.from != MaterialId::null)
    {
        if (toChangeId.to != MaterialId::null)
        {
            m_assetMetadataManager.move(toChangeId.from, toChangeId.to);
        }
        else
        {
            m_assetMetadataManager.remove(toChangeId.from);
        }
    }
}

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
    visitIfExists<engine::components::Camera>(v, e);
}

void UI::drawSelectedEntity()
{
    if (ImGui::Begin("Selected entity"))
    {
        if (m_selectedEntity.is_valid())
        {
            visitComponents([this](auto &component) {
                using ComponentType = decltype(component);
                flecs::component<ComponentType> componentEntity
                    = m_world.component<ComponentType>();
                if (ImGui::CollapsingHeader(
                    componentEntity.name().c_str(),
                    ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::PushID(componentEntity.name().c_str());
                    componentEdit(component);
                    ImGui::PopID();
                }
            }, m_selectedEntity);
        }
    }
    ImGui::End();
}

void UI::drawCameraParameters()
{
    if (ImGui::Begin("Camera parameters"))
    {
        ImGui::DragFloat3("Position", glm::value_ptr(m_cameraTransform.position), 0.25f);
        ImGui::DragFloat3("Rotation", glm::value_ptr(m_cameraTransform.rotation), 3.f, -180.f, 180.f);
        ImGui::SliderFloat("Fov", &m_camera.fov, 15.f, 120.f);
        ImGui::SliderFloat("Aspect", &m_camera.aspect, 0.1f, 3.f);
    }
    ImGui::End();
}

void UI::drawShadowMapsDebug()
{
    if (ImGui::Begin("Shadow maps"))
    {
        m_world.each([](flecs::entity e, engine::components::DirectionalLightShadowMap &directionalLightShadowMap) {
            Texture &texture =
                directionalLightShadowMap.shadowMapAtlasFramebuffer.getDepthAttachment();
            ImGui::BeginGroup();
            ImMe::Text("{} ({})", e.name().c_str(), e.id());
            ImGui::Image(
                (ImTextureID)(intptr_t)(texture.getId()),
                ImVec2(texture.getWidth() / 4.0f, texture.getHeight() / 4.0f),
                ImVec2(0, 1),
                ImVec2(1, 0));
            ImGui::EndGroup();
            ImGui::SameLine();
        });
    }
    ImGui::End();
}

void UI::drawCameraEntityViews()
{
    for (auto &cameraEntityView : m_cameraViews)
    {
        if (!cameraEntityView.entity.has<engine::components::Transform>()
            && !cameraEntityView.entity.has<engine::components::Camera>())
            continue;
        auto &transform = *cameraEntityView.entity.get_mut<engine::components::Transform>();
        auto &camera = *cameraEntityView.entity.get_mut<engine::components::Camera>();

        char windowName[128];
        const auto result = std::format_to_n(
            windowName,
            sizeof(windowName) - 1,
            "{}: Camera",
            cameraEntityView.entity.name().c_str());
        size_t textLength = static_cast<size_t>(result.size) < sizeof(windowName) - 1
            ? result.size
            : sizeof(windowName) - 1;
        windowName[textLength] = '\0';


        if (ImGui::Begin(windowName))
        {
            ImVec2 size = ImGui::GetContentRegionAvail();
            // resize when size changes
            if (size.x >= 1.0f && size.y >= 1.0f)
            {
                if (size.x != (float)cameraEntityView.framebuffer.getWidth()
                    || size.y != (float)cameraEntityView.framebuffer.getHeight())
                {
                    INFO(
                        "prev {} {} next {} {}",
                        cameraEntityView.framebuffer.getWidth(),
                        cameraEntityView.framebuffer.getHeight(),
                        size.x,
                        size.y);

                    cameraEntityView.framebuffer.shutdown();
                    Texture depthAttachment(Texture::Specification{
                        .width = static_cast<uint32_t>(size.x),
                        .height = static_cast<uint32_t>(size.y),
                        .format = Texture::Format::depth24,
                    });

                    std::vector<Texture> colorTextures;
                    colorTextures.emplace_back(Texture::Specification{
                        .width = static_cast<uint32_t>(size.x),
                        .height = static_cast<uint32_t>(size.y),
                        .format = Texture::Format::RGB8,
                        .linearMinification = false,
                        .linearMagnification = false,
                        .interpolateBetweenMipmapLevels = false,
                        .tile = false,
                    });
                    cameraEntityView.framebuffer.init({
                        .width = static_cast<uint32_t>(size.x),
                        .height = static_cast<uint32_t>(size.y),
                        .depthTextureAttachment = std::move(depthAttachment),
                        .colorTextureAttachments = std::move(colorTextures),
                    });
                    camera.aspect = size.x / size.y;
                }

                cameraEntityView.framebuffer.bind();
                GL::clear();
                m_renderer.renderWorld(transform, camera, m_world, m_assetManager, cameraEntityView.framebuffer);

                ImGui::Image(
                    (ImTextureID)(intptr_t)(cameraEntityView.framebuffer.getColorAttachments()[0].getId()),
                    size,
                    ImVec2(0, 1),
                    ImVec2(1, 0));
            }
        }
        ImGui::End();
    }
}

template<typename T>
constexpr std::underlying_type_t<T> &to_underlying_ref(T &value) noexcept
{
    return reinterpret_cast<std::underlying_type_t<T>&>(value);
}

void UI::componentEdit(engine::components::Transform &transform)
{
    ImMe::DragEditVec3("Position", transform.position, {0.0f, 0.0f, 0.0f}, 0.1f);
    ImMe::DragEditVec3("Rotation", transform.rotation, {0.0f, 0.0f, 0.0f}, 5.0f);
    ImMe::DragEditVec3("Scale",    transform.scale,    {1.0f, 1.0f, 1.0f}, 0.1f);
}

auto formatName(Texture::Format format)
{
    constexpr std::array formatToName = {
        "G8",
        "GA8",
        "RGB8",
        "RGBA8",
        "RGBA16F",
        "depth24stencil8",
        "depth24",
    };

    return formatToName[std::to_underlying(format)];
}

void UI::componentEdit(engine::components::Material &materialComponent)
{
    ImGui::BeginGroup();
    bool changed = ImMe::InputUInt("ID", &to_underlying_ref(materialComponent.id));

    const Material *material = m_assetManager.get(materialComponent.id);

    if (changed && !material)
        ERROR("Error getting material with id {}", std::to_underlying(materialComponent.id));

    if (!material)
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Material with this id does not exist");
        return;
    }

    for (auto &[textureType, name] : {
        std::pair{MaterialTextureType::diffuse, "Diffuse"},
        std::pair{MaterialTextureType::specular, "Specular"},
        std::pair{MaterialTextureType::normal, "Normal"},
        std::pair{MaterialTextureType::height, "Height"}
    }) {
        ImGui::PushID(textureType);
        bool isOpen = ImGui::TreeNodeEx(name);
        ImGui::SameLine();
        ImGui::Text("(%zu textures)", material->getTextureIds(textureType).size());
        ImGui::SameLine();
        ImGui::SmallButton("Add texture");
        if (isOpen)
        {
            if (ImGui::BeginTable("texture data", 2, ImGuiTableFlags_Resizable))
            {
                ImGui::TableSetupColumn("Image", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthStretch);

                for (TextureId id : material->getTextureIds(textureType))
                {
                    const Texture *texture = m_assetManager.get(id);
                    if (!texture) {
                        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error loading texture");
                        continue;
                    }

                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    if (ImGui::ImageButton(
                        std::to_string(std::to_underlying(id)).c_str(),
                        (ImTextureID)(intptr_t)(texture->getId()),
                        ImVec2(256, 256),
                        ImVec2(0, 1),
                        ImVec2(1, 0)))
                    {
                        INFO("Wants to select an image from filesystem to replace texture");
                    }

                    ImGui::TableSetColumnIndex(1);
                    ImGui::LabelText("Format", "%s", formatName(texture->getFormat()));
                    ImGui::LabelText("Size", "%ux%u", texture->getWidth(), texture->getHeight());

                }
                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
    ImGui::EndGroup();

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(MATERIAL_ID_DRAG_DROP_PAYLOAD_TYPE))
        {
            ASSERT(payload->DataSize == sizeof(MaterialId));
            materialComponent.id = *reinterpret_cast<MaterialId*>(payload->Data);
        }
        ImGui::EndDragDropTarget();
    }
}

void UI::componentEdit(engine::components::StaticMesh &staticMeshComponent)
{
    ImGui::BeginGroup();
    bool changed = ImMe::InputUInt("ID", &to_underlying_ref(staticMeshComponent.id));

    const StaticMesh *staticMesh = m_assetManager.get(staticMeshComponent.id);

    if (changed && !staticMesh)
        ERROR("Error getting static mesh with id {}", std::to_underlying(staticMeshComponent.id));

    if (!staticMesh)
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Static mesh with this id does not exist");
        return;
    }
    ImGui::EndGroup();

    if (ImGui::BeginDragDropTarget())
    {
        const ImGuiPayload *payload
            = ImGui::AcceptDragDropPayload(STATIC_MESH_ID_DRAG_DROP_PAYLOAD_TYPE);
        if (payload)
        {
            ASSERT(payload->DataSize == sizeof(StaticMeshId));
            staticMeshComponent.id = *reinterpret_cast<StaticMeshId*>(payload->Data);
        }
        ImGui::EndDragDropTarget();
    }
}

void UI::componentEdit(engine::components::AnimatedMesh &animatedMesh)
{
    ImMe::InputUInt("ID", &to_underlying_ref(animatedMesh.id));
}

void UI::componentEdit(engine::components::PointLight &pointLight)
{
    ImGui::ColorEdit3("Color", glm::value_ptr(pointLight.color));
    ImGui::SliderFloat("Intensity", &pointLight.intensity, 0.0f, 3.0f);
    ImGui::SliderFloat("Attenuation", &pointLight.attenuation, 0.0f, 0.1f);
}

void UI::componentEdit(engine::components::DirectionalLight &directionalLight)
{
    ImGui::ColorEdit3("Color", glm::value_ptr(directionalLight.color));
    ImGui::SliderFloat("Intensity", &directionalLight.intensity, 0.0f, 3.0f);
}

void UI::componentEdit(engine::components::Skybox &skybox)
{
    ImMe::InputUInt("ID", &to_underlying_ref(skybox.id));
}

void UI::componentEdit(engine::components::Camera &camera)
{
    ImGui::DragFloat("Aspect", &camera.aspect, 0.1f, 5.0f);
    ImGui::DragFloat("Fov", &camera.fov, 3.0f, 179.0f);
    ImGui::DragFloat("Near", &camera.near, 0.1f, 100.0f);
    ImGui::DragFloat("Far", &camera.far, 1.0f, 10000.0f);
}
