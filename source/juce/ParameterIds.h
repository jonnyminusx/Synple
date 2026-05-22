#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace parameter_id
{

constexpr int kVersion{1};

inline const juce::ParameterID glideMode{"glideMode", kVersion};
inline const juce::ParameterID glideRate{"glideRate", kVersion};
inline const juce::ParameterID glideBend{"glideBend", kVersion};
inline const juce::ParameterID filterFreq{"filterFreq", kVersion};
inline const juce::ParameterID filterReso{"filterReso", kVersion};
inline const juce::ParameterID filterEnv{"filterEnv", kVersion};
inline const juce::ParameterID filterLFO{"filterLFO", kVersion};
inline const juce::ParameterID filterVelocity{"filterVelocity", kVersion};
inline const juce::ParameterID filterAttack{"filterAttack", kVersion};
inline const juce::ParameterID filterDecay{"filterDecay", kVersion};
inline const juce::ParameterID filterSustain{"filterSustain", kVersion};
inline const juce::ParameterID filterRelease{"filterRelease", kVersion};
inline const juce::ParameterID envAttack{"envAttack", kVersion};
inline const juce::ParameterID envDecay{"envDecay", kVersion};
inline const juce::ParameterID envSustain{"envSustain", kVersion};
inline const juce::ParameterID envRelease{"envRelease", kVersion};
inline const juce::ParameterID lfoRate{"lfoRate", kVersion};
inline const juce::ParameterID vibrato{"vibrato", kVersion};
inline const juce::ParameterID noise{"noise", kVersion};
inline const juce::ParameterID octave{"octave", kVersion};
inline const juce::ParameterID tuning{"tuning", kVersion};
inline const juce::ParameterID outputLevel{"outputLevel", kVersion};
inline const juce::ParameterID polyMode{"polyMode", kVersion};
inline const juce::ParameterID pwmDepth{"pwmDepth", kVersion};
inline const juce::ParameterID osc1Volume{"osc1Volume", kVersion};
inline const juce::ParameterID osc2Volume{"osc2Volume", kVersion};
inline const juce::ParameterID osc1Tune{"osc1Tune", kVersion};
inline const juce::ParameterID osc2Tune{"osc2Tune", kVersion};
inline const juce::ParameterID osc1Fine{"osc1Fine", kVersion};
inline const juce::ParameterID osc2Fine{"osc2Fine", kVersion};
inline const juce::ParameterID osc1Waveform{"osc1Waveform", kVersion};
inline const juce::ParameterID osc2Waveform{"osc2Waveform", kVersion};
inline const juce::ParameterID osc1PulseWidth{"osc1PulseWidth", kVersion};
inline const juce::ParameterID osc2PulseWidth{"osc2PulseWidth", kVersion};

} // namespace parameter_id
