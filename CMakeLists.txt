# RUNE Plugin SDK - CMake Template
# 
# This is a template CMakeLists.txt for building RUNE plugins.
# Copy this file to your plugin project and modify as needed.
#
# Usage:
#   1. Create a new directory for your plugin
#   2. Copy this CMakeLists.txt and the sdk/include folder
#   3. Create your plugin source files
#   4. Build with: cmake -B build && cmake --build build
#
# Example directory structure:
#   my_plugin/
#     CMakeLists.txt (this file)
#     include/          (copy from sdk/include)
#     src/
#       my_plugin.cpp
#     plugin.json

cmake_minimum_required(VERSION 3.15)

# Set your plugin name here
project(example_plugin LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Plugin source files - add your source files here
set(PLUGIN_SOURCES
    src/example_plugin.cpp
)

# Create shared library
add_library(${PROJECT_NAME} SHARED ${PLUGIN_SOURCES})

# Define NODEPLUG_BUILDING to export symbols correctly
target_compile_definitions(${PROJECT_NAME} PRIVATE NODEPLUG_BUILDING)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Platform-specific settings
if(WIN32)
    # Windows: Set output name without 'lib' prefix
    set_target_properties(${PROJECT_NAME} PROPERTIES
        PREFIX ""
        OUTPUT_NAME "${PROJECT_NAME}"
    )
elseif(APPLE)
    # macOS: Use .dylib extension
    set_target_properties(${PROJECT_NAME} PROPERTIES
        PREFIX "lib"
        SUFFIX ".dylib"
    )
else()
    # Linux: Use .so extension
    set_target_properties(${PROJECT_NAME} PROPERTIES
        PREFIX "lib"
        SUFFIX ".so"
    )
endif()

# Set visibility for symbols
if(NOT MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE -fvisibility=hidden)
endif()

# Output to a 'dist' directory for easy packaging
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dist
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dist
)

# Copy plugin.json to dist directory
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/plugin.json)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/plugin.json
            ${CMAKE_CURRENT_SOURCE_DIR}/dist/plugin.json
        COMMENT "Copying plugin.json to dist"
    )
endif()

message(STATUS "Building RUNE plugin: ${PROJECT_NAME}")
message(STATUS "Output directory: ${CMAKE_CURRENT_SOURCE_DIR}/dist")

