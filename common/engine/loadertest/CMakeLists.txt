add_executable(loadertest
    main.cpp
)

target_link_libraries(loadertest
    resource-file-formats
    logger
    cgltf
    engine
)

set_target_properties(loadertest PROPERTIES
    EXPORT_COMPILE_COMMANDS ON
    CXX_STANDARD 23
)

add_test(
    NAME loadertest
    COMMAND $<TARGET_FILE:loadertest>
)
