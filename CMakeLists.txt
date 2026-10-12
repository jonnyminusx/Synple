cmake_minimum_required(VERSION 3.24)

project(Synple VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_XCODE_GENERATE_SCHEME OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set to OFF to skip the JUCE plugin and its GTK/WebKit requirements
option(BUILD_PLUGIN "Build the JUCE audio plugin" ON)
set(SYNPLE_ENGINE_SOURCE_DIR "" CACHE PATH "Path to a local Synple engine checkout")
set(SYNPLE_BUILD_TESTS OFF CACHE BOOL "Engine tests are run from the engine repository" FORCE)

# We're going to use CPM as our package manager
set(CPM_DOWNLOAD_VERSION 0.42.3)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if (NOT EXISTS "${CPM_DOWNLOAD_LOCATION}")
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD
        "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake"
        "${CPM_DOWNLOAD_LOCATION}"
        STATUS CPM_DOWNLOAD_STATUS
    )
    list(GET CPM_DOWNLOAD_STATUS 0 CPM_DOWNLOAD_RESULT)
    if (NOT CPM_DOWNLOAD_RESULT EQUAL 0)
        message(FATAL_ERROR "Could not download CPM.cmake: ${CPM_DOWNLOAD_STATUS}")
    endif()
endif ()

include("${CPM_DOWNLOAD_LOCATION}")

if (SYNPLE_ENGINE_SOURCE_DIR)
    get_filename_component(SYNPLE_ENGINE_SOURCE_DIR_REAL "${SYNPLE_ENGINE_SOURCE_DIR}" REALPATH)
    get_filename_component(SYNPLE_PROJECT_SOURCE_DIR_REAL "${CMAKE_CURRENT_SOURCE_DIR}" REALPATH)
    if (SYNPLE_ENGINE_SOURCE_DIR_REAL STREQUAL SYNPLE_PROJECT_SOURCE_DIR_REAL)
        message(FATAL_ERROR "SYNPLE_ENGINE_SOURCE_DIR must point to a separate engine checkout")
    endif()
    if (NOT EXISTS "${SYNPLE_ENGINE_SOURCE_DIR_REAL}/CMakeLists.txt")
        message(FATAL_ERROR "No CMakeLists.txt found at SYNPLE_ENGINE_SOURCE_DIR=${SYNPLE_ENGINE_SOURCE_DIR}")
    endif()
    add_subdirectory("${SYNPLE_ENGINE_SOURCE_DIR_REAL}" "${CMAKE_BINARY_DIR}/synple-engine")
else()
    CPMAddPackage(
        NAME SynpleEngine
        GITHUB_REPOSITORY jonnyminusx/Jape
        GIT_TAG 15fb141e1b9f30773811fe9e41dc8728b4493dad
    )
    set(SYNPLE_ENGINE_SOURCE_DIR_REAL "${SynpleEngine_SOURCE_DIR}")
endif()

foreach(ENGINE_TARGET IN ITEMS Jape::Math Jape::Midi Jape::Dsp Jape::Synth)
    if (NOT TARGET ${ENGINE_TARGET})
        message(FATAL_ERROR "The engine project must define the ${ENGINE_TARGET} target")
    endif()
endforeach()

# ─── clang-format ─────────────────────────────────────────────────────────────
find_program(CLANG_FORMAT clang-format)
if (CLANG_FORMAT)
    file(GLOB_RECURSE SYNPLE_FORMAT_SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/source/juce/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/source/juce/*.h"
    )
    set(FORMAT_CHECK_SOURCE_FILES ${SYNPLE_FORMAT_SOURCE_FILES})

    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -i ${SYNPLE_FORMAT_SOURCE_FILES}
        COMMENT "Formatting Synple sources..."
        VERBATIM
    )
    add_custom_target(format-check
        COMMAND ${CLANG_FORMAT} --dry-run --Werror ${FORMAT_CHECK_SOURCE_FILES}
        COMMENT "Checking Synple formatting..."
        VERBATIM
    )
else()
    message(STATUS "clang-format not found — skipping format target")
endif()

# Source files eligible for static analysis (JUCE-free layers only).
# Requires a full build (BUILD_PLUGIN=ON) so that compile_commands.json
# contains entries for all files, including those that depend on juce_audio_basics.
file(GLOB_RECURSE ANALYSIS_SOURCE_FILES
    "${SYNPLE_ENGINE_SOURCE_DIR_REAL}/source/dsp/*.cpp"
    "${SYNPLE_ENGINE_SOURCE_DIR_REAL}/source/midi/*.cpp"
    "${SYNPLE_ENGINE_SOURCE_DIR_REAL}/source/synth/*.cpp"
    "${SYNPLE_ENGINE_SOURCE_DIR_REAL}/source/math/*.cpp"
)

# ─── clang-tidy ───────────────────────────────────────────────────────────────
find_program(CLANG_TIDY clang-tidy)
if (CLANG_TIDY)
    set(TIDY_STAMP "${CMAKE_BINARY_DIR}/clang-tidy.stamp")

    add_custom_command(
        OUTPUT "${TIDY_STAMP}"
        COMMAND ${CLANG_TIDY} -p "${CMAKE_BINARY_DIR}" ${ANALYSIS_SOURCE_FILES}
        COMMAND ${CMAKE_COMMAND} -E touch "${TIDY_STAMP}"
        DEPENDS ${ANALYSIS_SOURCE_FILES}
        COMMENT "Running clang-tidy..."
        VERBATIM
    )

    add_custom_target(tidy DEPENDS "${TIDY_STAMP}")
else()
    message(STATUS "clang-tidy not found — skipping tidy target")
endif()

# ─── IWYU ─────────────────────────────────────────────────────────────────────
find_program(IWYU_TOOL NAMES iwyu_tool.py iwyu_tool)
if (IWYU_TOOL)
    set(IWYU_STAMP "${CMAKE_BINARY_DIR}/iwyu.stamp")

    add_custom_command(
        OUTPUT "${IWYU_STAMP}"
        COMMAND "${IWYU_TOOL}" -p "${CMAKE_BINARY_DIR}" ${ANALYSIS_SOURCE_FILES}
        COMMAND ${CMAKE_COMMAND} -E touch "${IWYU_STAMP}"
        DEPENDS ${ANALYSIS_SOURCE_FILES}
        COMMENT "Running include-what-you-use..."
        VERBATIM
    )

    add_custom_target(iwyu DEPENDS "${IWYU_STAMP}")
else()
    message(STATUS "iwyu_tool not found — skipping iwyu target")
endif()

# ─── Plugin ───────────────────────────────────────────────────────────────────
if (BUILD_PLUGIN)

# On Linux, pull in system library include paths that JUCE modules need
if (UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    pkg_check_modules(WEBKIT webkit2gtk-4.1)
    if (NOT WEBKIT_FOUND)
        pkg_check_modules(WEBKIT REQUIRED webkit2gtk-4.0)
    endif()
endif()

# Bring in JUCE
CPMAddPackage(
    NAME juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.13
)

set(SourceFiles
    source/juce/Parameters.cpp
    source/juce/Parameters.h
    source/juce/PluginEditor.cpp
    source/juce/PluginEditor.h
    source/juce/PluginProcessor.cpp
    source/juce/PluginProcessor.h
)

juce_add_plugin(${PROJECT_NAME}
        COMPANY_NAME "GhostwrittenDSP"
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        PLUGIN_MANUFACTURER_CODE Gstw
        PLUGIN_CODE Snpl
        FORMATS VST3 AU Standalone
        PRODUCT_NAME "Synple"
)

get_target_property(PRODUCT_NAME ${PROJECT_NAME} JUCE_PRODUCT_NAME)
get_target_property(COMPANY_NAME ${PROJECT_NAME} JUCE_COMPANY_NAME)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    JUCE_PRODUCT_NAME="${PRODUCT_NAME}"
    JUCE_COMPANY_NAME="${COMPANY_NAME}"
    JUCE_PRODUCT_VERSION="${PROJECT_VERSION}"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SourceFiles})
target_sources(${PROJECT_NAME} PRIVATE ${SourceFiles})

# Add source directory to include paths to avoid relative includes
target_include_directories(${PROJECT_NAME} BEFORE PRIVATE "${SYNPLE_ENGINE_SOURCE_DIR_REAL}/source")
target_include_directories(${PROJECT_NAME} PRIVATE source)

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        JUCE_WEB_BROWSER=1
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_core
        juce::juce_data_structures
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        WebViewFiles
        Jape::Synth
)

file(COPY "${CMAKE_BINARY_DIR}/_deps/juce-src/modules/juce_gui_extra/native/javascript/"
     DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/source/ui/src/lib/juce/")

# Folder where web UI data reside (Vite build output)
set(WEBVIEW_FILES_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source/ui/public")

# Zip WebView files
set(WEBVIEW_FILES_ZIP_NAME "webview_files.zip")
set(TARGET_WEBVIEW_FILES_ZIP_PATH "${CMAKE_BINARY_DIR}/${WEBVIEW_FILES_ZIP_NAME}")
set(PUBLIC_PATH "${WEBVIEW_FILES_SOURCE_DIR}")
cmake_path(GET PUBLIC_PATH PARENT_PATH WORKING_DIRECTORY)

find_program(NPM_EXECUTABLE npm REQUIRED)

add_custom_target(BuildWebUI
  COMMAND ${NPM_EXECUTABLE} run build
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/source/ui"
  COMMENT "Building React WebView UI..."
  VERBATIM
)

add_custom_target(ZipWebViewFiles
  COMMAND
  ${CMAKE_COMMAND} -E tar cvf
  "${TARGET_WEBVIEW_FILES_ZIP_PATH}"
  --format=zip
  "${PUBLIC_PATH}"
  BYPRODUCTS
  "${TARGET_WEBVIEW_FILES_ZIP_PATH}"
  WORKING_DIRECTORY
  "${WORKING_DIRECTORY}"
  COMMENT "Zipping WebView files..."
  VERBATIM
)

# Pass the prefix of the zipped files (e.g., "public/") to C++
cmake_path(GET PUBLIC_PATH FILENAME ZIPPED_FILES_PREFIX)
target_compile_definitions(${PROJECT_NAME} PRIVATE ZIPPED_FILES_PREFIX="${ZIPPED_FILES_PREFIX}/")

if (UNIX AND NOT APPLE)
    target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
        ${GTK3_INCLUDE_DIRS} ${WEBKIT_INCLUDE_DIRS})
endif()

# Package web UI sources as binary data
juce_add_binary_data(WebViewFiles
    HEADER_NAME WebViewFiles.h
    NAMESPACE webview_files
    SOURCES ${TARGET_WEBVIEW_FILES_ZIP_PATH}
)
add_dependencies(ZipWebViewFiles BuildWebUI)
add_dependencies(WebViewFiles ZipWebViewFiles)

endif() # BUILD_PLUGIN
