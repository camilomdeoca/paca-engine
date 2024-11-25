add_library(resource-file-formats INTERFACE)

target_include_directories(resource-file-formats INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(resource-file-formats
    INTERFACE
        logger
        asserts
        serializers
)
