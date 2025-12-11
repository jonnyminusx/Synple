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
    float phaseMax_{0.0f};

    float sin0_{0.0f};
    float sin1_{0.0f};
    float dsin_{0.0f};

    float dc_{0.0f};
};

} // namespace synth
