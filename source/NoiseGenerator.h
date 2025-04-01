#pragma once

#include <juce_core/juce_core.h>

class NoiseGenerator
{
  public:
    void reset();
    float nextValue();

  private:
    juce::Random random_;
};
