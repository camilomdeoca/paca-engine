#include "ComponentsEdit.hpp"

#include "CustomComponents.hpp"

#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <utility>

namespace ui {

void componentEdit(EditorContext &editorContext, engine::components::Transform &transform)
{
    DragEditVec3("Position", transform.position, {0.0f, 0.0f, 0.0f}, 0.1f);
    DragEditVec3("Rotation", transform.rotation, {0.0f, 0.0f, 0.0f}, 5.0f);
    DragEditVec3("Scale",    transform.scale,    {1.0f, 1.0f, 1.0f}, 0.1f);
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
    ImGui::InputUInt("ID", &materialComponent.id);
    const Material &material = editorContext.resourceManager.getMaterial(materialComponent.id);
    for (auto &[textureType, name] : {
        std::pair{MaterialTextureType::diffuse, "Diffuse"},
        std::pair{MaterialTextureType::specular, "Specular"},
        std::pair{MaterialTextureType::normal, "Normal"}
    }) {
        ImGui::PushID(textureType);
        bool isOpen = ImGui::TreeNodeEx(name);
        ImGui::SameLine();
        ImGui::Text("(%zu textures)", material.getTextureIds(textureType).size());
        ImGui::SameLine();
        ImGui::SmallButton("Add texture");
        if (isOpen)
        {
            if (ImGui::BeginTable("texture data", 2, ImGuiTableFlags_Resizable))
            {
                ImGui::TableSetupColumn("Image", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthStretch);

                for (TextureId id : material.getTextureIds(textureType))
                {
                    const Texture &texture = editorContext.resourceManager.getTexture(id);
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    if (ImGui::ImageButton(
                        std::to_string(id).c_str(),
                        (ImTextureID)(intptr_t)(texture.getId()),
                        ImVec2(256, 256),
                        ImVec2(0, 1),
                        ImVec2(1, 0)))
                    {
                        INFO("Wants to select an image from filesystem to replace texture");
                    }

                    ImGui::TableSetColumnIndex(1);
                    ImGui::LabelText("Format", "%s", formatName(texture.getFormat()));
                    ImGui::LabelText("Size", "%ux%u", texture.getWidth(), texture.getHeight());

                }
                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
}

void componentEdit(EditorContext &editorContext, engine::components::StaticMesh &staticMesh)
{
    ImGui::InputUInt("ID", &staticMesh.id);
}

void componentEdit(EditorContext &editorContext, engine::components::AnimatedMesh &animatedMesh)
{
    ImGui::InputUInt("ID", &animatedMesh.id);
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
    ImGui::InputUInt("ID", &skybox.id);
}

} // namespace ui

