#pragma once

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

    void setAttackMultiplier(const float attackMultiplier);
    void setDecayMultiplier(const float decayMultiplier);
    void setSustainLevel(const float sustainLevel);
    void setReleaseMultiplier(const float releaseMultiplier);

  private:
    float level_{0.0f};
    float target_{0.0f};
    float multiplier_{0.0f};

    float attackMultiplier_{0.0f};
    float decayMultiplier_{0.0f};
    float sustainLevel_{1.0f};
    float releaseMultiplier_{0.0f};
};

} // namespace synth
