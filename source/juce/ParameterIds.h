#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

struct ParameterIds
{
    static constexpr int kVersion{1};

    static inline const juce::ParameterID glideMode{"glideMode", kVersion};
    static inline const juce::ParameterID glideRate{"glideRate", kVersion};
    static inline const juce::ParameterID glideBend{"glideBend", kVersion};
    static inline const juce::ParameterID filterFreq{"filterFreq", kVersion};
    static inline const juce::ParameterID filterReso{"filterReso", kVersion};
    static inline const juce::ParameterID filterEnv{"filterEnv", kVersion};
    static inline const juce::ParameterID filterLFO{"filterLFO", kVersion};
    static inline const juce::ParameterID filterVelocity{"filterVelocity", kVersion};
    static inline const juce::ParameterID filterAttack{"filterAttack", kVersion};
    static inline const juce::ParameterID filterDecay{"filterDecay", kVersion};
    static inline const juce::ParameterID filterSustain{"filterSustain", kVersion};
    static inline const juce::ParameterID filterRelease{"filterRelease", kVersion};
    static inline const juce::ParameterID envAttack{"envAttack", kVersion};
    static inline const juce::ParameterID envDecay{"envDecay", kVersion};
    static inline const juce::ParameterID envSustain{"envSustain", kVersion};
    static inline const juce::ParameterID envRelease{"envRelease", kVersion};
    static inline const juce::ParameterID lfoRate{"lfoRate", kVersion};
    static inline const juce::ParameterID vibrato{"vibrato", kVersion};
    static inline const juce::ParameterID noise{"noise", kVersion};
    static inline const juce::ParameterID octave{"octave", kVersion};
    static inline const juce::ParameterID tuning{"tuning", kVersion};
    static inline const juce::ParameterID outputLevel{"outputLevel", kVersion};
    static inline const juce::ParameterID polyMode{"polyMode", kVersion};
    static inline const juce::ParameterID pwmDepth{"pwmDepth", kVersion};
    static inline const juce::ParameterID osc1Volume{"osc1Volume", kVersion};
    static inline const juce::ParameterID osc2Volume{"osc2Volume", kVersion};
    static inline const juce::ParameterID osc1Tune{"osc1Tune", kVersion};
    static inline const juce::ParameterID osc2Tune{"osc2Tune", kVersion};
    static inline const juce::ParameterID osc1Fine{"osc1Fine", kVersion};
    static inline const juce::ParameterID osc2Fine{"osc2Fine", kVersion};
    static inline const juce::ParameterID osc1Waveform{"osc1Waveform", kVersion};
    static inline const juce::ParameterID osc2Waveform{"osc2Waveform", kVersion};
    static inline const juce::ParameterID osc1PulseWidth{"osc1PulseWidth", kVersion};
    static inline const juce::ParameterID osc2PulseWidth{"osc2PulseWidth", kVersion};
};
