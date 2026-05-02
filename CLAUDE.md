# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Synple is a subtractive synthesizer JUCE plugin built with C++20. It targets VST3, AU, and Standalone formats and uses a WebView-based UI served from bundled HTML/JS assets.

## Build

The project uses CMake with CPM for dependency management. JUCE is fetched from GitHub master on first build.

### Linux system dependencies

On Ubuntu/Debian, install these before configuring cmake:

```bash
apt-get install -y \
    libasound2-dev \
    libgl-dev \
    libcurl4-openssl-dev \
    libgtk-3-dev \
    libxrandr-dev \
    libwebkit2gtk-4.1-dev
```

Note: Ubuntu 24.04+ ships `webkit2gtk-4.1`; older distros may need `libwebkit2gtk-4.0-dev` instead. The `CMakeLists.txt` handles both automatically.

### Configure and build

```bash
# Configure (first time or after CMakeLists changes)
cmake -B build

# Build
cmake --build build

# The plugin is copied automatically after build (COPY_PLUGIN_AFTER_BUILD TRUE)
```

### Testing

Tests use Catch2 v3 and live in `tests/`. The `SynthTests` binary links only pure-C++ sources — no JUCE, GTK, or WebKit required.

**Build tests only** (no system dependencies needed):
```bash
cmake -B build -DBUILD_PLUGIN=OFF
cmake --build build --target SynthTests
```

**Adding a test file** — two steps:
1. Create `tests/FooTest.cpp` (use `#include <catch2/catch_test_macros.hpp>` and `#include <catch2/catch_approx.hpp>`).
2. In `CMakeLists.txt`, add `tests/FooTest.cpp` and any new `.cpp` sources to the `add_executable(SynthTests ...)` block.

Only `source/synth/` and `source/midi/` sources are eligible — they are JUCE-free. Never add `source/juce/` files to `SynthTests`.

### WebView UI development

The UI is a React + TypeScript + Tailwind app in `source/ui/`. Source lives in `source/ui/src/`. Vite builds to `source/ui/public/`, which CMake then zips and bundles as binary data.

**First-time setup** — run once after checkout:
```bash
cd source/ui && npm install
```

CMake runs `npm run build` automatically as part of the plugin build. The JUCE JS bridge files are copied by CMake from the JUCE source into `source/ui/src/lib/juce/` — do not edit them manually.

**Hot-reload development**: switch to the dev server so you don't need to rebuild the plugin for UI changes:
1. In `PluginEditor.cpp`, comment out `webView_.goToURL(webView_.getResourceProviderRoot())` and uncomment `webView_.goToURL(localDevServerAddress)`.
2. Run `cd source/ui && npm run dev` (serves on port 8080 with HMR).
3. Reload the plugin window to pick up the dev server.

## Architecture

### Layer separation

```
source/juce/   — JUCE plugin layer (AudioProcessor, AudioProcessorEditor, Parameters)
source/synth/  — Pure synth engine (no JUCE dependencies except juce_audio_basics)
source/midi/   — MIDI message processing (MidiProcessor, MidiState, NoteHandler, CC type alias)
source/utils/  — Constants, ear protection utility
source/ui/     — WebView frontend (HTML/JS)
```

### Signal flow

1. `PluginProcessor` receives audio/MIDI from the host, splits the buffer by MIDI events, calls `Synth::render()`, and routes MIDI via `Synth::midiProcessor().process()`.
2. `Synth` manages up to 8 `Voice` instances (1 active in mono mode). Each voice contains two `Oscillator`s, a `Filter`, and two `Envelope`s (amplitude + filter). A shared `NoiseGenerator` feeds noise into the mix.
3. `Synth::updateLfo()` runs every 32 samples and drives vibrato, PWM depth, and filter modulation across all voices.
4. Output level is smoothed via `juce::LinearSmoothedValue` and monitored with a JUCE `BallisticsFilter` envelope follower.

### Parameter system

- `source/juce/Parameters` owns the `AudioProcessorValueTreeState` and exposes typed accessors (e.g. `envelope()`, `filterQ()`). Raw parameter pointers are stored privately; public accessors map raw values to usable units.
- `source/juce/ParameterIDs.h` defines all parameter IDs as `juce::ParameterID` constants — use these everywhere instead of string literals.
- `source/synth/Parameters.h` is a plain struct holding computed values that the synth engine reads each render block. `Synth` stores one instance; `PluginProcessor` maps from the JUCE parameter layer into it via `update()`.

### WebView ↔ C++ bridge

The editor uses JUCE's WebView integration:
- **Parameter sync**: `WebSliderRelay` and `WebComboBoxRelay` objects + their `WebXxxParameterAttachment` counterparts keep React state in sync with APVTS parameters automatically. React hooks in `source/ui/src/hooks/` wrap the JUCE JS API (`getSliderState`, `getComboBoxState`).
- **Resource requests**: `getResource()` in the editor serves files from the bundled zip.

### Presets

`source/synth/Presets` stores factory presets as arrays of normalised parameter values. `PluginProcessor` implements `getNumPrograms` / `setCurrentProgram` using this class.

## Code review guidelines

### Naming
- Classes/structs: PascalCase. Methods: camelCase.
- Member variables (private/protected): trailing underscore (`period_`, `voices_`). This includes `static constexpr` class constants (`maxNumVoices_`, `lfoMaxSamplesPerUpdate_`).
- File-scope or anonymous-namespace `constexpr` constants: no underscore (`piOver4`, `analog`, `silence`).
- `#pragma once` in every header; never `#ifndef` include guards.

### Include order (within each group, alphabetical is fine)
1. Project headers — relative paths, no prefix (`"Voice.h"`, `"midi/MidiProcessor.h"`)
2. Standard library — angle brackets (`<cstdint>`, `<array>`, `<numbers>`)
3. JUCE headers last (`<juce_audio_basics/juce_audio_basics.h>`)

### Class layout
Enforce this order inside every class/struct:
1. `public:` section first — constructor, core methods, interface overrides, then setters/getters/accessors.
2. `private:` section last — helper methods first, member variables at the bottom.

### Const-correctness and initialization
- Every method that does not mutate state must be marked `const`.
- Primitive parameters passed by value; complex/aggregate parameters passed by `const&`.
- No `#define` for constants — use `constexpr` (or `inline constexpr` for file/namespace scope).
- Member variables initialized in-class: use `{}` or `= value` syntax directly on the declaration.
- Brace initialization throughout: `float x{0.0f}`, not `float x = 0.0f` or `float x(0.0f)`.

### `auto`
Avoid unless the type is verbose and unambiguous from context (e.g. `auto buffer = audioBuffer.channelBuffer(0)`). Spell out types in arithmetic, DSP, and MIDI code.

### Namespaces
- All source-layer code belongs in a named namespace matching its layer: `synth::`, `midi::`, `parameter_id::`.
- File-local helpers and constants go in an anonymous `namespace { }` block (see `Voice.cpp`).
- No `using namespace` directives anywhere.

### Error handling
- Use `jassert()` for internal invariant violations — not exceptions, not `assert()`.
- Guard clauses with early return at system boundaries (e.g. index-range checks in `setCurrentProgram`).

### C++20 idioms
- Fixed-size collections: `std::array`. Non-owning views: `std::span`.
- Range-based `for` loops over index loops wherever the index is not needed.
- Math constants: `std::numbers::pi_v<float>` and friends, not hand-rolled literals or `M_PI`.
- Every JUCE-owned class must carry `JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR` at the bottom of its `private:` section.

### Comments and dividers
- Prefer self-documenting names over explanatory comments.
- Comment only when the *why* is non-obvious: MIDI status byte meanings, magic DSP coefficients, hardware/host quirks.
- `//=====` section dividers belong only in `source/juce/` processor headers (matching JUCE convention). Do not use them in synth or midi layer headers.
- No Doxygen/docstring blocks.

### Layer discipline
- `source/synth/` and `source/midi/` must stay JUCE-free (eligible for `SynthTests`). Never add a JUCE include to these layers.
- Parameter IDs must come from `parameter_id::` constants in `ParameterIDs.h` — never raw string literals.
- The synth engine reads parameters via `synth::Parameters` (a plain struct); do not reach from the synth layer back into `source/juce/`.
