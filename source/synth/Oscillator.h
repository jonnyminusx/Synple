#pragma once

namespace synth
{

class Oscillator
{
  public:
    void reset();
    void setAmplitude(const float amplitude);
    void setIncrement(const float increment);

    float nextSample();

  private:
    float amplitude_{0.0f};
    float increment_{0.0f};
    float phase_{0.0f};
};

} // namespace synth
