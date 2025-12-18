cmake_minimum_required(VERSION 3.15)

# RUNE Plugin SDK
#
# This directory provides the public C headers for developing RUNE plugins.
# It can be consumed in two primary ways:
#   1) add_subdirectory(rune_plugin_sdk) in your plugin project and then:
#        target_link_libraries(myplugin PRIVATE rune_plugin_sdk)
#   2) Install this directory and use find_package(RunePluginSDK CONFIG)

project(rune_plugin_sdk LANGUAGES C)

add_library(rune_plugin_sdk INTERFACE)

target_include_directories(rune_plugin_sdk
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Optional: also provide the old single-project template as a helper snippet.
# To use it, copy this file and the include/ directory into your own project
# and replace the example target with your plugin target.
option(RUNE_SDK_ENABLE_TEMPLATE "Enable building the legacy example_plugin template target" OFF)

if(RUNE_SDK_ENABLE_TEMPLATE)
    # Legacy example template for a single plugin project.
    project(example_plugin_template LANGUAGES CXX C)

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    set(PLUGIN_SOURCES
        src/example_plugin.cpp
    )

    add_library(${PROJECT_NAME} SHARED ${PLUGIN_SOURCES})

    target_compile_definitions(${PROJECT_NAME} PRIVATE NODEPLUG_BUILDING)

    target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    if(WIN32)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            PREFIX ""
            OUTPUT_NAME "${PROJECT_NAME}"
        )
    elseif(APPLE)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            PREFIX "lib"
            SUFFIX ".dylib"
        )
    else()
        set_target_properties(${PROJECT_NAME} PROPERTIES
            PREFIX "lib"
            SUFFIX ".so"
        )
    endif()

    if(NOT MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE -fvisibility=hidden)
    endif()

    set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dist
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dist
    )

    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/plugin.json)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/plugin.json
                ${CMAKE_CURRENT_SOURCE_DIR}/dist/plugin.json
            COMMENT "Copying plugin.json to dist"
        )
    endif()

    message(STATUS "Building RUNE example plugin template: ${PROJECT_NAME}")
    message(STATUS "Output directory: ${CMAKE_CURRENT_SOURCE_DIR}/dist")
endif()

# Install headers and interface target so external projects can use:
#   find_package(RunePluginSDK CONFIG)
install(
    DIRECTORY include/
    DESTINATION include
)

install(
    TARGETS rune_plugin_sdk
    EXPORT RunePluginSDKTargets
)

install(
    EXPORT RunePluginSDKTargets
    NAMESPACE Rune::
    DESTINATION lib/cmake/RunePluginSDK
)

