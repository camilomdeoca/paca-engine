add_library(resource-file-formats STATIC
    include/ResourceFileFormats.hpp
    ResourceFileFormats.cpp
    Serializer.cpp
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
)
target_include_directories(resource-file-formats PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
