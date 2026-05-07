#pragma once

#include "Oscillator.h"

namespace synth
{

class SawtoothOscillator : public Oscillator
{
  public:
    void reset() override;
    void setAmplitude(float amplitude) override;
    void setPeriod(float period) override;
    void setModulation(float modulation) override;

    void squareWave(const SawtoothOscillator& other, float newPeriod);
    float nextSample() override;

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
