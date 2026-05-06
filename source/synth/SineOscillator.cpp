#include "SineOscillator.h"

#include "math/constants.h"
#include <cmath>

namespace synth
{

void Oscillator::reset()
{
    phase_ = 0.0f;

    sin0 = amplitude_ * std::sin(math::tau * phase_);
    sin1 = amplitude_ * std::sin(math::tau * (phase_ + increment_));
    dsin = 2.0f * std::cos(increment_ * math::tau);
}

void Oscillator::setAmplitude(const float amplitude)
{
    amplitude_ = amplitude;
}

void Oscillator::setIncrement(const float increment)
{
    increment_ = increment;
}

float Oscillator::nextSample()
{
    float sinx = (dsin * sin0) - sin1;
    sin1 = sin0;
    sin0 = sinx;
    return sinx;
}

} // namespace synth
