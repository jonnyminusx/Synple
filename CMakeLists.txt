cmake_minimum_required(VERSION 3.24)

# Change to your project name
project(Synple VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)

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
set(CMAKE_XCODE_GENERATE_SCHEME OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# We're going to use CPM as our package manager to bring in JUCE
# Check to see if we have CPM installed already.  Bring it in if we don't.
set(CPM_DOWNLOAD_VERSION 0.34.0)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if (NOT EXISTS ${CPM_DOWNLOAD_LOCATION})
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION})
endif ()

include(${CPM_DOWNLOAD_LOCATION})

# Bring in JUCE locally
CPMAddPackage(
    NAME juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG origin/master
)

# Make sure you include any new source files here
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
        source/synth/Filter.h
        source/synth/Filter.cpp
        source/synth/GlideMode.h
        source/synth/NoiseGenerator.cpp
        source/synth/NoiseGenerator.h
        source/synth/Oscillator.cpp
        source/synth/Oscillator.h
        source/synth/Output.h
        source/synth/Output.cpp
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

# Change these to your own preferences
juce_add_plugin(${PROJECT_NAME}
        COMPANY_NAME morphonium
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        PLUGIN_MANUFACTURER_CODE Mphm
        PLUGIN_CODE Jx11
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

# How we want our SourceFiles to appear in our IDE
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SourceFiles})

# Make the SourceFiles buildable
target_sources(${PROJECT_NAME} PRIVATE ${SourceFiles})

# Add source directory to include paths to avoid relative includes
target_include_directories(${PROJECT_NAME} PRIVATE source)

# These are some toggleable options from the JUCE CMake API
target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        JUCE_WEB_BROWSER=1
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# JUCE libraries to bring into our project
target_link_libraries(${PROJECT_NAME}
        PUBLIC
        juce::juce_analytics
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

file( COPY "${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/juce-src/modules/juce_gui_extra/native/javascript/" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/source/ui/src/lib/juce/")

# Folder where web UI data reside (Vite build output)
set(WEBVIEW_FILES_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source/ui/public")

# Zip WebView files
set(WEBVIEW_FILES_ZIP_NAME "webview_files.zip")
set(TARGET_WEBVIEW_FILES_ZIP_PATH "${CMAKE_BINARY_DIR}/${WEBVIEW_FILES_ZIP_NAME}")
cmake_path(ABSOLUTE_PATH WEBVIEW_FILES_SOURCE_DIR NORMALIZE OUTPUT_VARIABLE PUBLIC_PATH )
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

