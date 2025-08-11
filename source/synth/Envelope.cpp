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

void Envelope::release()
{
    target_ = 0.0f;
    multiplier_ = releaseMultiplier_;
}

bool Envelope::isActive() const
{
    return level_ > silence;
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
    return level_;
}

} // namespace synth
