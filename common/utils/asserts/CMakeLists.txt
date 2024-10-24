add_library(asserts INTERFACE)

target_include_directories(asserts INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(asserts
    INTERFACE logger
)
