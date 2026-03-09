cmake_minimum_required(VERSION 3.28)
project(Discovery)

find_package(slang CONFIG HINTS "$ENV{VULKAN_SDK}/lib/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")
find_package(Vulkan REQUIRED)

file(GLOB_RECURSE sources src/*.cpp src/*.h)

add_executable(${PROJECT_NAME} ${sources})

target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)