#pragma once

#include "ADSR.h"

namespace synth
{

class Envelope
{
  public:
    float nextValue();
    float currentValue() const;

    void reset();
    void attack();
    void release();
    bool isActive() const;
    bool isInAttack() const;

    void nudgeLevelUp();

    void setLevel(const float level);
    void setTarget(const float target);
    void setMultiplier(const float multiplier);
    void setADSR(const ADSR& adsr);

  private:
    float level_{0.0f};
    float target_{0.0f};
    float multiplier_{0.0f};
    ADSR adsr_{};
};

} // namespace synth
