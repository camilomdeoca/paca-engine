add_executable(serializers-test1
    main.cpp
)

target_link_libraries(serializers-test1
    serializers
)

set_target_properties(serializers-test1 PROPERTIES
    EXPORT_COMPILE_COMMANDS ON
    CXX_STANDARD 23
)

add_test(
    NAME serializers-test1
    COMMAND $<TARGET_FILE:serializers-test1>
)
