#include "SineOscillator.h"

#include "math/constants.h"
#include <cmath>

namespace synth
{

void SineOscillator::reset()
{
    phase_ = 0.0f;

    sinCurrent_ = amplitude_ * std::sin(math::tau * phase_);
    // sinPrevious_ represents sin(phase - normalizedFreq_), i.e. one step back.
    // Using -normalizedFreq_ here so the first nextSample() returns sin(+freq),
    // giving a sine that rises from zero rather than one that dips negative.
    sinPrevious_ = amplitude_ * std::sin(-math::tau * normalizedFreq_);
    sinRecurrenceCoeff_ = 2.0f * std::cos(normalizedFreq_ * math::tau);
}

void SineOscillator::setAmplitude(const float amplitude)
{
    amplitude_ = amplitude;
}

void SineOscillator::setNormalizedFreq(const float normalizedFreq)
{
    normalizedFreq_ = normalizedFreq;
}

float SineOscillator::nextSample()
{
    const float sinx = (sinRecurrenceCoeff_ * sinCurrent_) - sinPrevious_;
    sinPrevious_ = sinCurrent_;
    sinCurrent_ = sinx;
    return sinx;
}

} // namespace synth
