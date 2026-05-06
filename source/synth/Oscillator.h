#pragma once

namespace synth
{

class Oscillator
{
  public:
    void reset();
    void setAmplitude(const float amplitude);
    void setPeriod(const float period);
    void setModulation(const float modulation);

    void squareWave(const Oscillator& other, const float newPeriod);
    float nextSample();

  private:
    float period_{0.0f};
    float modulation_{1.0f};
    float amplitude_{0.0f};

    float increment_{0.0f};
    float phase_{0.0f};
    float halfPhase_{0.0f};

    float sinCurrent_{0.0f};
    float sinPrevious_{0.0f};
    float sinRecurrenceCoeff_{0.0f};

    float dcOffset_{0.0f};
};

} // namespace synth
