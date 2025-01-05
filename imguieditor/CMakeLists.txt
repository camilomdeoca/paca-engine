find_package(Freetype)
find_package(SDL2)

add_executable(imguieditor
    Main.cpp
    App.cpp
    UI.cpp
    ui/ComponentsEdit.cpp
    ui/CustomComponents.cpp
)

message(STATUS "FREETYPE_LIBRARIES = ${FREETYPE_LIBRARIES}")

target_link_libraries(imguieditor
    PRIVATE
        engine
        resource-file-formats
        reflection
        opengl-wrapper
        GLX
        OpenGL
        GLEW
        logger
        asserts
        assimp
        stb_image
        imgui
        ImGuizmo
        ${FREETYPE_LIBRARIES}
        ${SDL2_LIBRARIES}
)
target_include_directories(imguieditor
    PRIVATE
        ${FREETYPE_INCLUDE_DIRS}
        ${SDL2_INCLUDE_DIRS}
)
