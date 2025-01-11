#include "CustomComponents.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>

namespace ImMe {

bool InputUInt(
    const char* label,
    unsigned int* v,
    unsigned int step,
    unsigned int step_fast,
    ImGuiInputTextFlags flags)
{
    // Hexadecimal input provided as a convenience but the flag name is awkward. Typically you'd use InputText() to parse your own data, if you want to handle prefixes.
    const char* format = (flags & ImGuiInputTextFlags_CharsHexadecimal) ? "%08X" : "%d";
    return ImGui::InputScalar(label, ImGuiDataType_U32, (void*)v, (void*)(step > 0 ? &step : NULL),
            (void*)(step_fast > 0 ? &step_fast : NULL), format, flags);
}

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

