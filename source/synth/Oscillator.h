#pragma once

namespace synth
{

class Oscillator
{
  public:
    void setAmplitude(const float amplitude);
    void setFrequency(const float frequency);
    void setPhaseOffset(const float phaseOffset);
    void setSampleRate(const float sampleRate);

    void reset();
    float nextSample();

  private:
    float amplitude_{0.0f};
    float frequency_{0.0f};
    float phaseOffset_{0.0f};
    float sampleRate_{0.0f};
    int sampleIndex_{0};
};

} // namespace synth
