cmake_minimum_required(VERSION 3.29)
project(my-engine CXX)

# Set C++ standard and optimization level
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG -Wall -Wpedantic")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

message(STATUS "CMAKE_CXX_COMPILER = ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS = ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "CMAKE_CXX_STANDARD = ${CMAKE_CXX_STANDARD}")
message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

enable_testing()

add_subdirectory(lib/fontatlas)

add_subdirectory(lib/stb_image)

add_subdirectory(common/utils)
add_subdirectory(common/resource-file-formats)
add_subdirectory(common/serializers)
add_subdirectory(common/opengl)
add_subdirectory(common/engine)

add_subdirectory(editor)
add_subdirectory(game)

