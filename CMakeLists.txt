cmake_minimum_required(VERSION 4.0)
project(Discovery)

set(CMAKE_CXX_STANDARD 20)

find_package(slang CONFIG HINTS "$ENV{VULKAN_SDK}/lib/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")
find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

file(GLOB_RECURSE sources src/*.cpp src/*.h)
add_executable(${PROJECT_NAME} ${sources}
        src/utils/buffer/buffer.cpp
        src/utils/buffer/buffer.h)

# Run shader compilation script to generate vert.spv and frag.spv
set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/res/shaders)
set(SHADER_COMPILE_SCRIPT ${SHADER_DIR}/compile.sh)
set(VERT_SPV ${SHADER_DIR}/vert.spv)
set(FRAG_SPV ${SHADER_DIR}/frag.spv)
set(SHADER_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/res/shaders)

# Create output directory
add_custom_command(
        TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_OUTPUT_DIR}
        COMMENT "Creating shader output directory..."
)

# Add a custom command to run the compile.sh script
add_custom_command(
        OUTPUT ${VERT_SPV} ${FRAG_SPV}
        COMMAND ${SHADER_COMPILE_SCRIPT}
        WORKING_DIRECTORY ${SHADER_DIR}
        DEPENDS ${SHADER_COMPILE_SCRIPT}
        COMMENT "Compiling shaders..."
        VERBATIM
)

# Copy the generated shader files to the binary directory at build time
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${VERT_SPV} ${SHADER_OUTPUT_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy ${FRAG_SPV} ${SHADER_OUTPUT_DIR}/
        COMMENT "Copying compiled shaders to binary directory..."
)

# Make the executable depend on the shader compilation
add_custom_target(compile_shaders ALL
        DEPENDS ${VERT_SPV} ${FRAG_SPV}
        COMMENT "Ensuring shaders are compiled..."
)

# Add dependency so shaders are compiled before the executable builds
add_dependencies(${PROJECT_NAME} compile_shaders)

target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan glfw glm::glm)