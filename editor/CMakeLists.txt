
add_executable(editor
    ModelConverter.cpp
    CubemapConverter.cpp
    Main.cpp
)

target_link_libraries(editor
    resource-file-formats
    stb_image
    logger
    asserts
    assimp
)

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)
qt_standard_project_setup()
qt_add_executable(editor-qt
    Editor.cpp
    MainWindow.cpp
    FilepathEdit.cpp
    CameraView.cpp
    ModelConverter.cpp
    CubemapConverter.cpp
    AssetPackEditor.cpp
)
target_link_libraries(editor-qt
    PRIVATE Qt6::Widgets Qt6::OpenGLWidgets
    resource-file-formats
    opengl-wrapper
    GL
    GLEW
    logger
    asserts
    assimp
    stb_image
)
