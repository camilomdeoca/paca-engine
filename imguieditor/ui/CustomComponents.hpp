#pragma once

#include <imgui.h>
#include <glm/glm.hpp>

namespace ui {

bool DragEditVec3(
    const char *label,
    glm::vec3 &v,
    const glm::vec3 &resetValue = {0.0f, 0.0f, 0.0f},
    float v_speed = 1.0f,
    float v_min = 0.0f,
    float v_max = 0.0f,
    const char *format = "%.3f",
    ImGuiSliderFlags flags = 0);

} // namespace ui
