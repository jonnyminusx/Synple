# Synple

A subtractive synthesizer plugin built with C++20 and JUCE, with a React/TypeScript WebView UI. Originally inspired by the book *Creating Synthesizer Plug-Ins with C++ and JUCE*.

Targets VST3, AU, and Standalone formats.

<img width="876" height="285" alt="image" src="https://github.com/user-attachments/assets/f6fc386b-fe35-4dfb-b07e-47060ca6b630" />

## Features

- **8-voice polyphony** with mono/poly mode switching and voice stealing
- **Dual oscillators** per voice with coarse and fine tuning
- **Resonant low-pass filter** with key tracking, velocity sensitivity, and envelope/LFO modulation
- **Two ADSR envelopes** per voice — amplitude and filter
- **LFO** for vibrato and PWM modulation
- **Portamento/glide** with Off, Glide, and Legato modes
- **Noise generator** mixed into the signal chain
- MIDI: velocity, pitch bend, mod wheel, sustain pedal, aftertouch, filter CCs
- 53 factory presets (pads, leads, bass, synth, FX)

## Building

**Linux dependencies:**

```bash
apt-get install -y libasound2-dev libgl-dev libcurl4-openssl-dev \
    libgtk-3-dev libxrandr-dev libwebkit2gtk-4.1-dev
```

**Build:**

```bash
cd source/ui && npm install && cd ../..
cmake -B build
cmake --build build
```

JUCE is fetched automatically via CPM on first configure. The plugin is copied to the system plugin directory after each build.

## UI Development

The UI lives in `source/ui/` (React + TypeScript + Tailwind, built with Vite). CMake bundles the Vite output as binary data inside the plugin.

For hot-reload during development:

1. In `PluginEditor.cpp`, swap the production URL for `localDevServerAddress`.
2. Run `cd source/ui && npm run dev` (port 8080, HMR enabled).
3. Reload the plugin window.

## Architecture

```
source/juce/   — JUCE plugin layer (AudioProcessor, AudioProcessorEditor, Parameters)
source/synth/  — Pure synth engine (no JUCE dependencies)
source/midi/   — MIDI processing (MidiProcessor, MidiState, NoteHandler)
source/dsp/    — DSP primitives (Filter, AudioBuffer, noise)
source/math/   — Math constants
source/ui/     — React/TypeScript WebView frontend
```

`PluginProcessor` splits audio buffers at MIDI event boundaries and calls `Synth::render()`. The synth manages up to 8 `Voice` instances, each with two oscillators, a filter, and two envelopes. The LFO updates every 32 samples. Parameters flow from JUCE's APVTS through a plain `synth::Parameters` struct that the engine reads each block.
