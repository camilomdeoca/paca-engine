cmake_minimum_required(VERSION 3.27)
project(new_engine CXX)

# Set C++ standard and optimization level
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -stdlib=libc++")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG -Wall -Wpedantic")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")

message(STATUS "CMAKE_CXX_COMPILER = ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

# Include directories
include_directories(
    src
    src/vendor/stb_image/
)

# Set build path
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build)

# Add executable
file(GLOB_RECURSE SOURCES src/*.cpp )
add_executable(${PROJECT_NAME}
    ${SOURCES}
)

add_subdirectory(lib/fontatlas)
add_subdirectory(lib/paca-tools)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    SDL2
    GL
    GLEW
    png
    assimp
    fontatlas
    pacaread
)
