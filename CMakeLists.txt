cmake_minimum_required(VERSION 3.24)

project(Synple VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_XCODE_GENERATE_SCHEME OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set to OFF to build only the test suite (no GTK/WebKit/JUCE required)
option(BUILD_PLUGIN "Build the JUCE audio plugin" ON)
option(ENABLE_SANITIZERS "Build SynthTests with ASan and UBSan" OFF)

# We're going to use CPM as our package manager
set(CPM_DOWNLOAD_VERSION 0.34.0)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if (NOT EXISTS "${CPM_DOWNLOAD_LOCATION}")
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake" "${CPM_DOWNLOAD_LOCATION}")
endif ()

include("${CPM_DOWNLOAD_LOCATION}")

# ─── clang-format ─────────────────────────────────────────────────────────────
find_program(CLANG_FORMAT clang-format)
if (CLANG_FORMAT)
    file(GLOB_RECURSE FORMAT_SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/source/dsp/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/source/dsp/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/source/juce/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/source/juce/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/source/midi/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/source/midi/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/source/synth/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/source/synth/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/source/math/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/source/math/*.h"
    )

    set(FORMAT_STAMP "${CMAKE_BINARY_DIR}/clang-format.stamp")

    add_custom_command(
        OUTPUT "${FORMAT_STAMP}"
        COMMAND ${CLANG_FORMAT} -i ${FORMAT_SOURCE_FILES}
        COMMAND ${CMAKE_COMMAND} -E touch "${FORMAT_STAMP}"
        DEPENDS ${FORMAT_SOURCE_FILES}
        COMMENT "Running clang-format..."
        VERBATIM
    )

    add_custom_target(format ALL DEPENDS "${FORMAT_STAMP}")
else()
    message(STATUS "clang-format not found — skipping format target")
endif()

# Source files eligible for static analysis (JUCE-free layers only).
# Requires a full build (BUILD_PLUGIN=ON) so that compile_commands.json
# contains entries for all files, including those that depend on juce_audio_basics.
file(GLOB_RECURSE ANALYSIS_SOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/source/dsp/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/midi/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/synth/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/math/*.cpp"
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
    tests/AudioBufferTest.cpp
    tests/FilterTest.cpp
    tests/MidiProcessorTest.cpp
    tests/OscillatorTest.cpp
    source/dsp/AudioBuffer.cpp
    source/dsp/Filter.cpp
    source/dsp/NoiseGenerator.cpp
    source/midi/MidiProcessor.cpp
    source/synth/Oscillator.cpp
)

target_include_directories(SynthTests PRIVATE source)
target_link_libraries(SynthTests PRIVATE Catch2::Catch2WithMain)

if (ENABLE_SANITIZERS)
    target_compile_options(SynthTests PRIVATE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
        -fno-sanitize-recover=all
    )
    target_link_options(SynthTests PRIVATE -fsanitize=address,undefined)
endif()

include("${Catch2_SOURCE_DIR}/extras/Catch.cmake")
catch_discover_tests(SynthTests)

add_custom_command(
    TARGET SynthTests
    POST_BUILD
    COMMAND "$<TARGET_FILE:SynthTests>"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Running unit tests..."
    VERBATIM
)

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
    source/dsp/AudioBuffer.cpp
    source/dsp/AudioBuffer.h
    source/dsp/NoiseGenerator.cpp
    source/dsp/NoiseGenerator.h
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
    source/dsp/Filter.cpp
    source/dsp/Filter.h
    source/dsp/Goertzel.h
    source/synth/ADSR.h
    source/synth/Envelope.cpp
    source/synth/Envelope.h
    source/synth/GlideMode.h
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
    source/math/constants.h
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
