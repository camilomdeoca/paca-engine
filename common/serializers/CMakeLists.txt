add_library(serializers INTERFACE)

target_include_directories(serializers INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(serializers
    INTERFACE
        logger
        asserts
        yaml-cpp
)
