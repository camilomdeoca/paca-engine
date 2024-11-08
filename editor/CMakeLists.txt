find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)
qt_standard_project_setup()
qt_add_executable(editor
    Editor.cpp
    MainWindow.cpp
    FilepathEdit.cpp
    CameraView.cpp
    ModelConverter.cpp
    CubemapConverter.cpp
    FontConverter.cpp
    AssetPackEditor.cpp
)
target_link_libraries(editor
    PRIVATE Qt6::Widgets Qt6::OpenGLWidgets
    resource-file-formats
    serializers
    opengl-wrapper
    GL
    GLEW
    logger
    asserts
    assimp
    stb_image
    freetype
)

add_subdirectory(tests)
