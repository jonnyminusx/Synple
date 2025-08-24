#include "Envelope.h"
#include <cmath>

namespace synth
{

namespace
{
constexpr float silence = 0.0001f;
}

void Envelope::reset()
{
    level_ = 0.0f;
    target_ = 0.0f;
    multiplier_ = 0.0f;
}

void Envelope::attack()
{
    level_ += silence + silence;
    target_ = 2.0f;
    multiplier_ = attackMultiplier_;
}

void Envelope::release()
{
    target_ = 0.0f;
    multiplier_ = releaseMultiplier_;
}

bool Envelope::isActive() const
{
    return level_ > silence;
}

bool Envelope::isInAttack() const
{
    return target_ >= 2.0f;
}

void Envelope::setLevel(const float level)
{
    level_ = level;
}

void Envelope::setTarget(const float target)
{
    target_ = target;
}

void Envelope::setMultiplier(const float multiplier)
{
    multiplier_ = multiplier;
}

void Envelope::setAttackMultiplier(const float attackMultiplier)
{
    attackMultiplier_ = attackMultiplier;
}

void Envelope::setDecayMultiplier(const float decayMultiplier)
{
    decayMultiplier_ = decayMultiplier;
}

void Envelope::setSustainLevel(const float sustainLevel)
{
    sustainLevel_ = sustainLevel;
}

void Envelope::setReleaseMultiplier(const float releaseMultiplier)
{
    releaseMultiplier_ = releaseMultiplier;
}

float Envelope::nextValue()
{
    level_ = multiplier_ * (level_ - target_) + target_;

    if (level_ + target_ > 3.0f)
    {
        multiplier_ = decayMultiplier_;
        target_ = sustainLevel_;
    }

    return level_;
}

float Envelope::currentValue() const
{
    return level_;
}

} // namespace synth
