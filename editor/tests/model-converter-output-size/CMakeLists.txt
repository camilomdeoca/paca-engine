add_executable(model-converter-output-size-test
    ../../ModelConverter.cpp
    main.cpp
)

target_link_libraries(model-converter-output-size-test
    resource-file-formats
    serializers
    stb_image
    assimp
)

set_target_properties(model-converter-output-size-test PROPERTIES
    EXPORT_COMPILE_COMMANDS ON
    CXX_STANDARD 23
)

add_test(
    NAME model-comverter-output-size-test
    COMMAND $<TARGET_FILE:model-converter-output-size-test>
)
