#pragma once

#include <format>
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/glm.hpp>

namespace ImMe {

bool InputUInt(const char* label, unsigned int* v, unsigned int step = 1,
        unsigned int step_fast = 100, ImGuiInputTextFlags flags = 0);

bool DragEditVec3(
    const char *label,
    glm::vec3 &v,
    const glm::vec3 &resetValue = {0.0f, 0.0f, 0.0f},
    float v_speed = 1.0f,
    float v_min = 0.0f,
    float v_max = 0.0f,
    const char *format = "%.3f",
    ImGuiSliderFlags flags = 0);

namespace detail {

// based in ImFormatStringToTempBufferV function
template<class ...Args>
void formatStringToTempBuffer(const char** out_buf, const char** out_buf_end, std::format_string<Args...> fmt, Args &&...args)
{
    ImGuiContext& g = *GImGui;

    // Have to subtract 1 from the buffer size because this replaces vsnprintf wich writes up to
    // buffer_size - 1 characters
    const std::format_to_n_result result
        = std::format_to_n(g.TempBuffer.Data, g.TempBuffer.Size - 1, fmt, std::forward<Args>(args)...);
    
    //int buf_len = ImFormatStringV(g.TempBuffer.Data, g.TempBuffer.Size, fmt, args);
    size_t buf_len = result.size;
    if (g.TempBuffer.Data == NULL)
        buf_len = result.size;
    if (result.size >= g.TempBuffer.Size)
        buf_len = g.TempBuffer.Size - 1;

    g.TempBuffer.Data[buf_len] = '\0'; // This string doesnt need to be null terminated because it
                                       // will be used with ImGui::TextUnformatted() wich takes
                                       // begin and end pointers but in case i use it for something
                                       // else

    *out_buf = g.TempBuffer.Data;
    if (out_buf_end) { *out_buf_end = g.TempBuffer.Data + buf_len; }
}

}

template<class ...Args>
void Text(std::format_string<Args...> fmt, Args &&...args)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    const char* text, *text_end;
    detail::formatStringToTempBuffer(&text, &text_end, fmt, std::forward<Args>(args)...);
    ImGui::TextEx(text, text_end, ImGuiTextFlags_NoWidthForLargeClippedText);
}

} // namespace ui
