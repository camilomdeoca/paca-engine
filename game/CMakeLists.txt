
include_directories(
    .
)

file(GLOB_RECURSE SOURCES *.cpp )
add_executable(game
    ${SOURCES}
)

target_link_libraries(game
    SDL2
    GL
    GLEW
    engine
    opengl-wrapper
    assimp
    fontatlas
    pacaread
    resource-file-formats
    stb_image
)
