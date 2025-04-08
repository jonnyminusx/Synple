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

void Oscillator::setAmplitude(const float amplitude)
{
    amplitude_ = amplitude;
}

void Oscillator::setFrequency(const float frequency)
{
    frequency_ = frequency;
}

void Oscillator::setPhaseOffset(const float phaseOffset)
{
    phaseOffset_ = phaseOffset;
}

void Oscillator::setSampleRate(const float sampleRate)
{
    sampleRate_ = sampleRate;
}

void Oscillator::reset()
{
    sampleIndex_ = 0;
}

float Oscillator::nextSample()
{
    const float output{amplitude_ * std::sinf((tau * frequency_ * sampleIndex_ / sampleRate_) + phaseOffset_)};
    ++sampleIndex_;
    return output;
}

} // namespace synth
