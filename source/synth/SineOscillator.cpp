#include "SineOscillator.h"

#include <cmath>
#include <numbers>

namespace synth
{

namespace
{

constexpr float pi = std::numbers::pi_v<float>;
constexpr float tau = 2.0f * pi;

} // namespace

void Oscillator::reset()
{
    phase_ = 0.0f;

    sin0 = amplitude_ * std::sinf(tau * phase_);
    sin1 = amplitude_ * std::sinf(tau * (phase_ + increment_));
    dsin = 2.0f * std::cosf(increment_ * tau);
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
