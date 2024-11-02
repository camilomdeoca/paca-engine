add_library(engine STATIC
    AnimatedMesh.cpp
    Animation.cpp
    Font.cpp
    Light.cpp
    Material.cpp
    Model.cpp
    OrthoCamera.cpp
    PerspectiveCamera.cpp
    Renderer2D.cpp
    Renderer.cpp
    ResourceManager.cpp
    StaticMesh.cpp
)

target_include_directories(engine PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
    $<INSTALL_INTERFACE:/>
    ${CMAKE_CURRENT_SOURCE_DIR}
    include
)

target_include_directories(engine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(engine
    opengl-wrapper
    logger
    asserts
    assimp
    fontatlas
    resource-file-formats
    stb_image
)
