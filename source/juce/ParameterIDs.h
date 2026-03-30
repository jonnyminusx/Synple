#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace parameter_id
{

#define PARAMETER_ID(str) const juce::ParameterID str(#str, 1);

PARAMETER_ID(oscMix)
PARAMETER_ID(oscTune)
PARAMETER_ID(oscFine)
PARAMETER_ID(glideMode)
PARAMETER_ID(glideRate)
PARAMETER_ID(glideBend)
PARAMETER_ID(filterFreq)
PARAMETER_ID(filterReso)
PARAMETER_ID(filterEnv)
PARAMETER_ID(filterLFO)
PARAMETER_ID(filterVelocity)
PARAMETER_ID(filterAttack)
PARAMETER_ID(filterDecay)
PARAMETER_ID(filterSustain)
PARAMETER_ID(filterRelease)
PARAMETER_ID(envAttack)
PARAMETER_ID(envDecay)
PARAMETER_ID(envSustain)
PARAMETER_ID(envRelease)
PARAMETER_ID(lfoRate)
PARAMETER_ID(vibrato)
PARAMETER_ID(noise)
PARAMETER_ID(octave)
PARAMETER_ID(tuning)
PARAMETER_ID(outputLevel)
PARAMETER_ID(polyMode)

#undef PARAMETER_ID

} // namespace parameter_id
