add_executable(loadertest
    main.cpp
)
add_executable(animationloadertest
    animationloadertest.cpp
)

target_link_libraries(loadertest
    resource-file-formats
    logger
    cgltf
    engine
)
target_link_libraries(animationloadertest
    resource-file-formats
    logger
    cgltf
    engine
)

set_target_properties(loadertest PROPERTIES
    EXPORT_COMPILE_COMMANDS ON
    CXX_STANDARD 23
)
set_target_properties(animationloadertest PROPERTIES
    EXPORT_COMPILE_COMMANDS ON
    CXX_STANDARD 23
)

add_test(
    NAME loadertest
    COMMAND $<TARGET_FILE:loadertest>
)
add_test(
    NAME animationloadertest
    COMMAND $<TARGET_FILE:loadertest>
)
