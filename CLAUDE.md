# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Synple is a subtractive synthesizer JUCE plugin built with C++20. It targets VST3, AU, and Standalone formats and uses a WebView-based UI served from bundled HTML/JS assets.

## Build

The project uses CMake with CPM for dependency management. JUCE is fetched from GitHub master on first build.

```bash
# Configure (first time or after CMakeLists changes)
cmake -B build

# Build
cmake --build build

# The plugin is copied automatically after build (COPY_PLUGIN_AFTER_BUILD TRUE)
```

There are no tests currently. Suggested testing tools from TODO.md: pluginval, auval (macOS native), and thread sanitizer.

### WebView UI development

The UI files in `source/ui/public/` are zipped at build time into `webview_files.zip` and bundled as binary data. During development you can point the webview at a local dev server instead:

- In `PluginEditor.cpp`, comment out `webView_.goToURL(webView_.getResourceProviderRoot())` and uncomment `webView_.goToURL(localDevServerAddress)` (pointing to `http://127.0.0.1:8080`).
- Serve `source/ui/public/` on port 8080.

The JUCE JS bridge files are copied from the JUCE source into `source/ui/public/js/juce/` by CMake — do not edit them manually.

## Architecture

### Layer separation

```
source/juce/   — JUCE plugin layer (AudioProcessor, AudioProcessorEditor, Parameters)
source/synth/  — Pure synth engine (no JUCE dependencies except juce_audio_basics)
source/midi/   — MIDI CC type alias
source/utils/  — Constants, ear protection utility
source/ui/     — WebView frontend (HTML/JS)
```

### Signal flow

1. `PluginProcessor` receives audio/MIDI from the host, splits the buffer by MIDI events, and calls `Synth::render()` and `Synth::midiMessage()`.
2. `Synth` manages up to 8 `Voice` instances (1 active in mono mode). Each voice contains two `Oscillator`s, a `Filter`, and two `Envelope`s (amplitude + filter). A shared `NoiseGenerator` feeds noise into the mix.
3. `Synth::updateLfo()` runs every 32 samples and drives vibrato, PWM depth, and filter modulation across all voices.
4. Output level is smoothed via `juce::LinearSmoothedValue` and monitored with a JUCE `BallisticsFilter` envelope follower, whose value is exposed to the WebView as `outputLevel.json`.

### Parameter system

- `source/juce/Parameters` owns the `AudioProcessorValueTreeState` and exposes typed accessors (e.g. `envelope()`, `filterQ()`). Raw parameter pointers are stored privately; public accessors map raw values to usable units.
- `source/juce/ParameterIDs.h` defines all parameter IDs as `juce::ParameterID` constants — use these everywhere instead of string literals.
- `source/synth/Parameters.h` is a plain struct holding computed values that the synth engine reads each render block. `Synth` stores one instance; `PluginProcessor` maps from the JUCE parameter layer into it via `update()`.

### WebView ↔ C++ bridge

The editor uses JUCE's WebView integration:
- **C++ → JS**: `webView_.emitEventIfBrowserIsVisible(eventId, data)` fires named events; the JS side listens with `window.__JUCE__.backend.addEventListener`.
- **JS → C++**: `Juce.getNativeFunction("nativeFunction")` calls a handler registered with `.withNativeFunction(...)` in the editor constructor.
- **Parameter sync**: `WebSliderRelay`, `WebToggleButtonRelay`, and `WebComboBoxRelay` objects + their `WebXxxParameterAttachment` counterparts keep the JS UI controls in sync with APVTS parameters automatically via the JUCE JS API (`Juce.getSliderState`, `Juce.getToggleState`, `Juce.getComboBoxState`).
- **Resource requests**: `getResource()` in the editor serves files from the bundled zip and also handles ad-hoc JSON endpoints (`outputLevel.json`, `data.json`).

### Presets

`source/synth/Presets` stores factory presets as arrays of normalised parameter values. `PluginProcessor` implements `getNumPrograms` / `setCurrentProgram` using this class.
