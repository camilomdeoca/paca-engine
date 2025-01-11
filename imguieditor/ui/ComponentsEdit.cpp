#include "ComponentsEdit.hpp"

#include "CustomComponents.hpp"
#include "engine/IdTypes.hpp"

#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <utility>

namespace ui {

template<typename T>
constexpr std::underlying_type_t<T> &to_underlying_ref(T &value) noexcept
{
    return reinterpret_cast<std::underlying_type_t<T>&>(value);
}

void componentEdit(EditorContext &editorContext, engine::components::Transform &transform)
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

void componentEdit(EditorContext &editorContext, engine::components::Material &materialComponent)
{
    bool changed = ImMe::InputUInt("ID", &to_underlying_ref(materialComponent.id));

    const Material *material = editorContext.resourceManager.get(materialComponent.id);

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
        std::pair{MaterialTextureType::normal, "Normal"}
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
                    const Texture *texture = editorContext.resourceManager.get(id);
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
}

void componentEdit(EditorContext &editorContext, engine::components::StaticMesh &staticMeshComponent)
{
    bool changed = ImMe::InputUInt("ID", &to_underlying_ref(staticMeshComponent.id));
    const StaticMesh *staticMesh = editorContext.resourceManager.get(staticMeshComponent.id);

    if (changed && !staticMesh)
        ERROR("Error getting static mesh with id {}", std::to_underlying(staticMeshComponent.id));

    if (!staticMesh)
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Static mesh with this id does not exist");
        return;
    }
}

void componentEdit(EditorContext &editorContext, engine::components::AnimatedMesh &animatedMesh)
{
    ImMe::InputUInt("ID", &to_underlying_ref(animatedMesh.id));
}

void componentEdit(EditorContext &editorContext, engine::components::PointLight &pointLight)
{
    ImGui::SliderFloat3("Color", glm::value_ptr(pointLight.color), 0.0f, 1.0f);
    ImGui::SliderFloat("Intensity", &pointLight.intensity, 0.0f, 3.0f);
    ImGui::SliderFloat("Attenuation", &pointLight.attenuation, 0.0f, 0.1f);
}

void componentEdit(
    EditorContext &editorContext,
    engine::components::DirectionalLight &directionalLight)
{
    ImGui::SliderFloat3("Color", glm::value_ptr(directionalLight.color), 0.0f, 1.0f);
    ImGui::SliderFloat("Intensity", &directionalLight.intensity, 0.0f, 3.0f);
}

void componentEdit(EditorContext &editorContext, engine::components::Skybox &skybox)
{
    ImMe::InputUInt("ID", &to_underlying_ref(skybox.id));
}

} // namespace ui

