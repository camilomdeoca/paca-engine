add_library(ImGuizmo STATIC
    ImGuizmo/ImGuizmo.cpp
)

target_include_directories(ImGuizmo
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo>
        $<INSTALL_INTERFACE:/ImGuizmo>
)

target_link_libraries(ImGuizmo
    PRIVATE
    imgui
)
