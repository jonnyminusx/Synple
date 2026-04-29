# Synple

A subtractive synthesizer plugin for VST3, AU, and Standalone formats, built with C++20 and JUCE. The UI is a React/TypeScript web app served via JUCE's WebView integration. Originally inspired by the book *Creating Synthesizer Plug-Ins with C++ and JUCE*.

---

## Features

### Synth Engine

- **8-voice polyphony** with intelligent voice stealing (lowest-level voice is replaced first)
- **Mono/poly mode** — switch between monophonic (single voice with note queue) and full 8-voice polyphony
- **Dual oscillators** per voice — mix between oscillator 1 and oscillator 2, with coarse and fine tuning
- **Noise generator** — variable white noise mixed into the signal chain
- **State-variable filter** — resonant low-pass filter with key tracking and velocity sensitivity
- **Dual ADSR envelopes** — independent amplitude and filter envelopes per voice
- **LFO** — sine wave LFO with vibrato (pitch) and PWM modulation targets
- **Portamento/glide** — three glide modes (Off, Glide, Legato) with adjustable rate and bend amount
- **Output protection** — ear-safety limiter to prevent digital clipping

### Parameters

| Section | Parameter | Description |
|---|---|---|
| Oscillator | Osc Mix | Blend between oscillator 1 and 2 |
| | Osc Tune | Coarse pitch offset (semitones) |
| | Osc Fine | Fine pitch offset (±50 cents) |
| Glide | Glide Mode | Off / Glide / Legato |
| | Glide Rate | Speed of pitch glide |
| | Glide Bend | Pitch bend amount at note transitions |
| Filter | Filter Freq | Cutoff frequency |
| | Filter Reso | Resonance (Q) |
| | Filter Env | Filter envelope modulation depth |
| | Filter LFO | LFO modulation depth on filter cutoff |
| | Filter Velocity | Velocity-to-cutoff sensitivity |
| | Filter Key Track | Cutoff follows MIDI note pitch |
| Amp Envelope | Attack, Decay, Sustain, Release | Amplitude ADSR |
| Filter Envelope | Attack, Decay, Sustain, Release | Filter cutoff ADSR |
| LFO | LFO Rate | LFO frequency |
| | Vibrato | LFO-to-pitch modulation depth |
| Global | Noise Mix | White noise level |
| | Octave | Global octave shift |
| | Tuning | Master pitch offset (±50 cents) |
| | Output Level | Output gain (dB) |
| | Poly Mode | Mono / Poly voice mode |

### MIDI

- Note on/off with velocity
- Sustain pedal (CC #64) — holds notes until pedal release
- Pitch bend wheel
- Mod wheel (CC #1) — controls vibrato and PWM depth
- Channel pressure (aftertouch) — modulates filter cutoff
- Filter cutoff CC (CC #74)
- Filter resonance CC (CC #71)
- Mono mode note queue — held notes are recalled in last-note priority order

### Factory Presets

53 factory presets organised by category:

- **Pads** — 5th Sweep Pad, Echo Pad, Space Chimes, Solid Backing, Velocity Backing, Very Soft Pad
- **Leads** — 808 State Lead, Detuned Techno Lead, Hard Lead, Bubble, Monosynth, Moogcury Lite, Gangsta Whine, Higher Synth
- **Bass** — Mono Glide, Analog Bass, Low Pulses, Sine Infra-Bass, Wobble Bass, Squelch Bass, Rubber Bass, Soft Pick Bass, Fretless Bass, 303 Saw Bass, 303 Square Bass
- **Synth** — Whistler, Pizzicato, Synth Strings, Leslie Organ, Click Organ, Hard Organ, Bass Clarinet, Trumpet, Soft Horn, Brass Section, Synth Brass, Detuned Syn Brass, Power PWM, Water Velocity, Ghost, Soft E.Piano, Thumb Piano, Steel Drums
- **FX** — Car Horn, Helicopter, Arctic Wind, Thip, Synth Tom, Squelchy Frog

---

## Building

### Prerequisites

**Linux (Ubuntu/Debian):**

```bash
apt-get install -y \
    libasound2-dev \
    libgl-dev \
    libcurl4-openssl-dev \
    libgtk-3-dev \
    libxrandr-dev \
    libwebkit2gtk-4.1-dev
```

> Ubuntu 24.04+ ships `webkit2gtk-4.1`. Older distros may need `libwebkit2gtk-4.0-dev` instead — `CMakeLists.txt` detects both automatically.

**Node.js** (any recent LTS) is required to build the web UI.

### Configure and build

```bash
# First-time UI setup
cd source/ui && npm install && cd ../..

# Configure (downloads JUCE via CPM on first run)
cmake -B build

# Build plugin and UI
cmake --build build
```

The plugin is copied to the system plugin directory automatically after a successful build (`COPY_PLUGIN_AFTER_BUILD TRUE`).

### Output formats

| Format | Platform |
|---|---|
| VST3 | Windows, macOS, Linux |
| AU | macOS only |
| Standalone | All platforms |

---

## UI Development

The UI is a **React + TypeScript + Tailwind** app located in `source/ui/`. Vite builds it to `source/ui/public/`, which CMake zips and embeds as binary data in the plugin binary.

### Hot-reload workflow

To iterate on the UI without rebuilding the plugin each time:

1. In `source/juce/PluginEditor.cpp`, comment out the production URL line and uncomment the dev server line (`localDevServerAddress`).
2. Start the dev server:
   ```bash
   cd source/ui && npm run dev
   ```
   The server runs on port 8080 with HMR.
3. Reload the plugin window in your DAW or standalone app to connect to the dev server.

---

## Architecture

```
source/juce/   — JUCE plugin layer (AudioProcessor, AudioProcessorEditor, Parameters)
source/synth/  — Pure synth engine (no JUCE dependencies except juce_audio_basics)
source/midi/   — MIDI processing (MidiProcessor, MidiState, NoteHandler)
source/utils/  — Constants, ear protection
source/ui/     — React/TypeScript WebView frontend
```

### Signal flow

1. `PluginProcessor` receives audio/MIDI from the host, splits the buffer at MIDI event boundaries, and calls `Synth::render()`.
2. `Synth` manages up to 8 `Voice` instances. Each voice owns two `Oscillators`, a `Filter`, and two `Envelopes` (amplitude + filter). A shared `NoiseGenerator` feeds into the mix.
3. `Synth::updateLfo()` runs every 32 samples and drives vibrato, PWM depth, and filter modulation across all active voices.
4. Output level is smoothed via `juce::LinearSmoothedValue` and monitored with a JUCE `BallisticsFilter` envelope follower.

### Parameter system

- `source/juce/Parameters` owns the `AudioProcessorValueTreeState` and exposes typed accessors. All parameter IDs are defined as `juce::ParameterID` constants in `source/juce/ParameterIDs.h`.
- `source/synth/Parameters.h` is a plain struct of computed values that the synth engine reads each render block. `PluginProcessor` maps from the JUCE parameter layer into it each block via `update()`.

### WebView bridge

- `WebSliderRelay` / `WebComboBoxRelay` objects and their `WebXxxParameterAttachment` counterparts keep React state in sync with APVTS parameters automatically.
- React hooks in `source/ui/src/hooks/` wrap the JUCE JS bridge API (`getSliderState`, `getComboBoxState`).
- `getResource()` in the editor serves files from the bundled zip.

---

## Project info

| | |
|---|---|
| Version | 0.0.1 |
| Company | morphonium |
| Manufacturer code | `Mphm` |
| Plugin code | `Jx11` |
| C++ standard | C++20 |
| Build system | CMake + CPM |
| UI framework | React 18, TypeScript, Tailwind CSS, Vite |
