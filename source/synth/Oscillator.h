#pragma once

namespace synth
{

class Oscillator
{
  public:
    void reset();
    void setAmplitude(const float amplitude);
    void setPeriod(const float period);

    float nextSample();

  private:
    float period_{0.0f};
    float amplitude_{0.0f};

    float increment_{0.0f};
    float phase_{0.0f};
    float phaseMax_{0.0f};
};

} // namespace synth
