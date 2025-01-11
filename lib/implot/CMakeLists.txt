add_library(implot STATIC
    implot/implot.cpp
    implot/implot_items.cpp
)

target_include_directories(implot
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/implot
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/implot>
        $<INSTALL_INTERFACE:/implot>
)

target_link_libraries(implot
    PRIVATE
    imgui
)
