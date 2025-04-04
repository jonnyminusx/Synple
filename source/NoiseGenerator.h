#pragma once

#include "synth/INoiseGenerator.h"
#include <juce_core/juce_core.h>

class NoiseGenerator : public synth::INoiseGenerator
{
  public:
    void reset();
    float nextValue();

  private:
    juce::Random random_;
};
