#include "Envelope.h"
#include <cmath>

namespace synth
{

namespace
{
constexpr float silence{0.0001f};
}

void Envelope::noteOn()
{
    multiplier_ = newMultiplier_;
}

void Envelope::setLevel(const float level)
{
    level_ = level;
}

void Envelope::setDecay(const float decaySamples)
{
    newMultiplier_ = decaySamples <= 0.0f ? 1.0f : std::exp(std::log(silence) / decaySamples);
}

float Envelope::nextValue()
{
    level_ *= multiplier_;
    return level_;
}

} // namespace synth
