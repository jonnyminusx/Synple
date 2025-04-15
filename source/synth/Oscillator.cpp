#include "Oscillator.h"

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
    phase_ = 0;
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
    phase_ += increment_;
    if (phase_ >= 1.0f)
    {
        phase_ -= 1.0f;
    }

    return amplitude_ * std::sinf(tau * phase_);
}

} // namespace synth
