# Timer Plugin - Example RUNE Plugin
cmake_minimum_required(VERSION 3.15)
project(timer_plugin LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PLUGIN_SOURCES
    src/timer_plugin.cpp
)

add_library(${PROJECT_NAME} SHARED ${PLUGIN_SOURCES})

target_compile_definitions(${PROJECT_NAME} PRIVATE NODEPLUG_BUILDING)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" OUTPUT_NAME "${PROJECT_NAME}")
elseif(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "lib" SUFFIX ".dylib")
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "lib" SUFFIX ".so")
endif()

if(NOT MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE -fvisibility=hidden)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dist
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dist
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/plugin.json
        ${CMAKE_CURRENT_SOURCE_DIR}/dist/plugin.json
)

