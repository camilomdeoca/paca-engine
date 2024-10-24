add_library(opengl-wrapper STATIC
    gl.cpp
    VertexBuffer.cpp
    IndexBuffer.cpp
    VertexArray.cpp
    Texture.cpp
    Shader.cpp
    FrameBuffer.cpp
)

target_include_directories(opengl-wrapper PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
    $<INSTALL_INTERFACE:/>
    ${CMAKE_CURRENT_SOURCE_DIR}
    include
)

target_include_directories(opengl-wrapper PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(opengl-wrapper
    GL
    GLEW
    stb_image
    logger
    asserts
)
