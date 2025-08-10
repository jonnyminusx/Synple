#pragma once

namespace synth
{

class Envelope
{
  public:
    void noteOn();
    float nextValue();

    void setLevel(const float level);
    void setDecay(const float decaySamples);

  private:
    float level_{1.0f};
    float multiplier_{0.999f};
    float newMultiplier_{0.999f};
};

} // namespace synth
