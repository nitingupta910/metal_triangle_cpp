cmake_minimum_required(VERSION 3.15)
project(metal_triangle)

set(CMAKE_CXX_STANDARD 20)

# Find the SDL2 framework
find_package(SDL2 REQUIRED)

# Include SDL2 and metal-cpp in the project
include_directories(${SDL2_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/metal-cpp)

# Find Metal framework (macOS)
find_library(METAL_FRAMEWORK Metal REQUIRED)
find_library(METAL_KIT_FRAMEWORK MetalKit REQUIRED)
find_library(QUARTZ_CORE_FRAMEWORK QuartzCore REQUIRED)
find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)

# Add the executable
add_executable(metal_triangle main.cpp)

# Name of the compiled Metal library
set(METAL_SHADER_NAME shaders.metallib)

# Path to the Metal shader source
set(METAL_SHADER_SRC ${CMAKE_SOURCE_DIR}/shaders.metal)

# Output paths for the intermediate .air and final .metallib files
set(METAL_SHADER_AIR ${CMAKE_BINARY_DIR}/shaders.air)
set(METAL_SHADER_LIB ${CMAKE_BINARY_DIR}/${METAL_SHADER_NAME})

# Add a custom command to compile the Metal shaders
add_custom_command(
        OUTPUT ${METAL_SHADER_LIB}
        COMMAND xcrun metal -c ${METAL_SHADER_SRC} -o ${METAL_SHADER_AIR}
        COMMAND xcrun metallib ${METAL_SHADER_AIR} -o ${METAL_SHADER_LIB}
        DEPENDS ${METAL_SHADER_SRC}
        COMMENT "Compiling Metal shaders"
)

# Add a custom target to ensure the shaders are always compiled
add_custom_target(compile_shaders ALL DEPENDS ${METAL_SHADER_LIB})

# Ensure the shaders are available for your application
add_dependencies(metal_triangle compile_shaders)

# Copy the .metallib file to the runtime build directory
add_custom_command(
        TARGET compile_shaders POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${METAL_SHADER_LIB} ${CMAKE_BINARY_DIR}
        COMMENT "Copying Metal library file to build directory"
)

# Link SDL2 and Metal frameworks
target_link_libraries(metal_triangle PRIVATE
        ${SDL2_LIBRARIES}
        ${METAL_FRAMEWORK}
        ${METAL_KIT_FRAMEWORK}
        ${QUARTZ_CORE_FRAMEWORK}
        ${FOUNDATION_FRAMEWORK}
)
