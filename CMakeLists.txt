cmake_minimum_required(VERSION 3.29)
project(my-engine CXX)

# Set C++ standard and optimization level
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG -Wall -Wpedantic")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++abi")

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

if(NOT WIN32)
  string(ASCII 27 Esc)
  set(ColorReset "${Esc}[m")
  set(Cyan        "${Esc}[36m")
endif()

message(STATUS "${Cyan}CMAKE_BUILD_TYPE${ColorReset} = ${CMAKE_BUILD_TYPE}")
message(STATUS "${Cyan}CMAKE_CXX_STANDARD${ColorReset} = ${CMAKE_CXX_STANDARD}")
message(STATUS "${Cyan}CMAKE_CXX_COMPILER${ColorReset} = ${CMAKE_CXX_COMPILER}")
message(STATUS "${Cyan}CMAKE_CXX_COMPILER_ID${ColorReset} = ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "${Cyan}CMAKE_C_STANDARD${ColorReset} = ${CMAKE_C_STANDARD}")
message(STATUS "${Cyan}CMAKE_C_COMPILER${ColorReset} = ${CMAKE_C_COMPILER}")
message(STATUS "${Cyan}CMAKE_C_COMPILER_ID${ColorReset} = ${CMAKE_C_COMPILER_ID}")

enable_testing()

# add_subdirectory(lib/fontatlas)
add_subdirectory(lib/stb_image)
add_subdirectory(lib/flecs EXCLUDE_FROM_ALL)
add_subdirectory(lib/imgui EXCLUDE_FROM_ALL)
add_subdirectory(lib/ImGuizmo EXCLUDE_FROM_ALL)
add_subdirectory(lib/implot EXCLUDE_FROM_ALL)
add_subdirectory(lib/cgltf EXCLUDE_FROM_ALL)

set(NFD_PORTAL on)
add_subdirectory(lib/nativefiledialog-extended)

add_subdirectory(common/utils)
add_subdirectory(common/resource-file-formats)
add_subdirectory(common/reflection)
add_subdirectory(common/opengl)
add_subdirectory(common/engine)
add_subdirectory(editor)
add_subdirectory(imguieditor)
add_subdirectory(game)

