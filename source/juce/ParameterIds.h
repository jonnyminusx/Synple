#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

struct ParameterIds
{
    static constexpr int kVersion{1};

    static inline const juce::ParameterID oscMix{"oscMix", kVersion};
    static inline const juce::ParameterID oscTune{"oscTune", kVersion};
    static inline const juce::ParameterID oscFine{"oscFine", kVersion};
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
    static inline const juce::ParameterID oscWaveform{"oscWaveform", kVersion};
};
