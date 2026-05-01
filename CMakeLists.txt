cmake_minimum_required(VERSION 3.24)

project(Synple VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_XCODE_GENERATE_SCHEME OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set to OFF to build only the test suite (no GTK/WebKit/JUCE required)
option(BUILD_PLUGIN "Build the JUCE audio plugin" ON)

# We're going to use CPM as our package manager
set(CPM_DOWNLOAD_VERSION 0.34.0)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if (NOT EXISTS "${CPM_DOWNLOAD_LOCATION}")
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake" "${CPM_DOWNLOAD_LOCATION}")
endif ()

include("${CPM_DOWNLOAD_LOCATION}")

# ─── Tests ────────────────────────────────────────────────────────────────────
# The test binary depends only on pure-C++ synth sources; no JUCE or GTK needed.
enable_testing()
include(CTest)

CPMAddPackage(
    NAME Catch2
    VERSION 3.5.2
    GITHUB_REPOSITORY catchorg/Catch2
)

add_executable(SynthTests
    tests/OscillatorAliasingTest.cpp
    source/synth/Oscillator.cpp
)

target_include_directories(SynthTests PRIVATE source)
target_link_libraries(SynthTests PRIVATE Catch2::Catch2WithMain)

include("${Catch2_SOURCE_DIR}/extras/Catch.cmake")
catch_discover_tests(SynthTests)

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
    include_directories(SYSTEM ${GTK3_INCLUDE_DIRS} ${WEBKIT_INCLUDE_DIRS})
endif()

# Bring in JUCE
CPMAddPackage(
    NAME juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG origin/master
)

set(SourceFiles
    source/juce/Parameters.cpp
    source/juce/Parameters.h
    source/juce/PluginEditor.cpp
    source/juce/PluginEditor.h
    source/juce/PluginProcessor.cpp
    source/juce/PluginProcessor.h
    source/midi/CC.h
    source/midi/MidiProcessor.cpp
    source/midi/MidiProcessor.h
    source/midi/MidiState.h
    source/midi/NoteHandler.h
    source/synth/ADSR.h
    source/synth/AudioBuffer.cpp
    source/synth/AudioBuffer.h
    source/synth/Envelope.cpp
    source/synth/Envelope.h
    source/synth/Filter.cpp
    source/synth/Filter.h
    source/synth/GlideMode.h
    source/synth/NoiseGenerator.cpp
    source/synth/NoiseGenerator.h
    source/synth/Oscillator.cpp
    source/synth/Oscillator.h
    source/synth/Output.cpp
    source/synth/Output.h
    source/synth/Parameters.h
    source/synth/Preset.h
    source/synth/Presets.cpp
    source/synth/Presets.h
    source/synth/SineOscillator.cpp
    source/synth/SineOscillator.h
    source/synth/Synth.cpp
    source/synth/Synth.h
    source/synth/Voice.cpp
    source/synth/Voice.h
    source/utils/constants.h
    source/utils/protectYourEars.cpp
    source/utils/protectYourEars.h
)

juce_add_plugin(${PROJECT_NAME}
        COMPANY_NAME "Ghostwritten DSP"
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
target_include_directories(${PROJECT_NAME} PRIVATE source)

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        JUCE_WEB_BROWSER=1
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(${PROJECT_NAME}
        PUBLIC
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
        PRIVATE
        WebViewFiles
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

# Package web UI sources as binary data
juce_add_binary_data(WebViewFiles
    HEADER_NAME WebViewFiles.h
    NAMESPACE webview_files
    SOURCES ${TARGET_WEBVIEW_FILES_ZIP_PATH}
)
add_dependencies(ZipWebViewFiles BuildWebUI)
add_dependencies(WebViewFiles ZipWebViewFiles)

endif() # BUILD_PLUGIN
