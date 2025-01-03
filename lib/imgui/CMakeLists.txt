find_package(SDL2)

add_library(imgui STATIC
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_sdl2.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui
        ${SDL2_INCLUDE_DIRS}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imgui>
        $<INSTALL_INTERFACE:/imgui>
)

target_link_libraries(imgui
    PRIVATE
    ${SDL2_LIBRARIES}
)
