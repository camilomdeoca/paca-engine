add_library(resource-file-formats STATIC
    ResourceFileFormats.cpp
)
#add_serializer_generation(resource-file-formats
#    HEADER Serializer.hpp
#    CODE Serializer.cpp
#    FILES ResourceFileFormats.cpp
#)
target_include_directories(resource-file-formats PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
    $<INSTALL_INTERFACE:/>
    ${CMAKE_CURRENT_SOURCE_DIR}
    include
)
target_include_directories(resource-file-formats PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(resource-file-formats
    logger
    asserts
    yaml-cpp
)
