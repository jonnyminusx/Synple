#pragma once

#include <juce_core/juce_core.h>

namespace synth
{

class NoiseGenerator
{
  public:
    void reset();
    float nextValue();

  private:
    juce::Random random_;
};

} // namespace synth
