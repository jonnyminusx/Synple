#pragma once

namespace synth
{

class SineOscillator
{
  public:
    void reset();
    void setAmplitude(const float amplitude);
    void setNormalizedFreq(const float normalizedFreq);

    float nextSample();

  private:
    float amplitude_{0.0f};
    float normalizedFreq_{0.0f};
    float phase_{0.0f};

    float sinCurrent_{0.0f};
    float sinPrevious_{0.0f};
    float sinRecurrenceCoeff_{0.0f};
};

} // namespace synth
