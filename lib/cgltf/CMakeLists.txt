add_library(cgltf STATIC
    cgltf.cpp
)

target_include_directories(cgltf SYSTEM
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/cgltf>
        $<INSTALL_INTERFACE:/cgltf>
)
