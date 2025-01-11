add_library(engine STATIC
    AnimatedMesh.cpp
    Animation.cpp
    Font.cpp
    Material.cpp
    OrthoCamera.cpp
    PerspectiveCamera.cpp
    #Renderer2D.cpp
    #Renderer.cpp
    ForwardRenderer.cpp
    NewResourceManager.cpp
    StaticMesh.cpp
    #SceneManager.cpp
    Action.cpp
    Input.cpp
    Window.cpp
    Components.cpp
    Loader.cpp
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
    cgltf
    resource-file-formats
    reflection
    stb_image
    flecs::flecs_static
    yaml-cpp
)

add_subdirectory(loadertest)
