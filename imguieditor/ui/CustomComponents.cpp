#include "CustomComponents.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>

namespace ui {

bool DragEditVec3(
    const char *label,
    glm::vec3 &v,
    const glm::vec3 &resetValue,
    float v_speed,
    float v_min,
    float v_max,
    const char *format,
    ImGuiSliderFlags flags)
{
    bool valueChanged
        = ImGui::DragFloat3(label, glm::value_ptr(v), v_speed, v_min, v_max, format, flags);

    if (ImGui::BeginPopupContextItem(label))
    {
        if (ImGui::Selectable("reset")) v = resetValue;
        ImGui::EndPopup();
    }

    return valueChanged;
}

} // namespace ui

