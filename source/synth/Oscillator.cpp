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
    phase_ = 0.0f;
    phaseBandLimited_ = -0.5f;
}

void Oscillator::setAmplitude(const float amplitude)
{
    amplitude_ = amplitude;
}

void Oscillator::setFrequency(const float frequency)
{
    frequency_ = frequency;
}

void Oscillator::setIncrement(const float increment)
{
    increment_ = increment;
}

void Oscillator::setSampleRate(const float sampleRate)
{
    sampleRate_ = sampleRate;
}

float Oscillator::nextBandLimitedSample()
{
    phaseBandLimited_ += increment_;

    if (phaseBandLimited_ >= 1.0f)
    {
        phaseBandLimited_ -= 1.0f;
    }

    const float nyquist{sampleRate_ / 2.0f};
    float output{0.0f};
    float harmonicFrequency{frequency_};
    float i{1.0f};
    float m{2.0f / pi};

    while (harmonicFrequency < nyquist)
    {
        output += m * std::sinf(tau * phaseBandLimited_ * i) / i;
        harmonicFrequency += frequency_;
        i += 1.0f;
        m = -m;
    }

    return output;
}

float Oscillator::nextSample()
{
    return nextBandLimitedSample() * amplitude_;
}

} // namespace synth
