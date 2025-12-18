cmake_minimum_required(VERSION 3.16)
project(env_plugin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define we're building a plugin
add_definitions(-DNODEPLUG_BUILDING)

# Find SDK include directory
set(SDK_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../include")

# Create shared library
add_library(${PROJECT_NAME} SHARED
    src/env_plugin.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${SDK_INCLUDE_DIR}
)

# Windows-specific settings
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS OFF
        PREFIX ""
    )
endif()

# Output to dist folder
set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dist"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dist"
)

# Copy plugin.json to dist
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/dist/plugin.json"
)

