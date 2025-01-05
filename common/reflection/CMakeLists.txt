add_library(reflection INTERFACE)

target_include_directories(reflection INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(reflection
    INTERFACE
        logger
        asserts
)

add_subdirectory(tests)
