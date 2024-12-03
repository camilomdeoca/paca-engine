find_package(Freetype)
find_package(Qt6 COMPONENTS Widgets OpenGLWidgets REQUIRED)
qt_standard_project_setup(REQUIRES 6)
qt_add_executable(editor
    Editor.cpp
    MainWindow.cpp
    FilepathEdit.cpp
    CameraView.cpp
    ModelConverter.cpp
    CubemapConverter.cpp
    FontConverter.cpp
    AssetPackEditor.cpp
    CombineAssetPack.cpp
)
message(STATUS "CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}")
message(STATUS "FREETYPE_LIBRARIES = ${FREETYPE_LIBRARIES}")
target_link_libraries(editor
    PRIVATE
        Qt6::Widgets
        Qt6::OpenGLWidgets
        resource-file-formats
        serializers
        opengl-wrapper
        GLX
        OpenGL
        GLEW
        logger
        asserts
        assimp
        stb_image
        ${FREETYPE_LIBRARIES}
)
target_include_directories(editor PRIVATE ${FREETYPE_INCLUDE_DIRS})

#add_subdirectory(tests)
