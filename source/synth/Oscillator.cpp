#include "Oscillator.h"

#include <cmath>
#include <numbers>

namespace synth
{

namespace
{

constexpr float pi{std::numbers::pi_v<float>};
constexpr float quarterPi{pi / 4.0f};

} // namespace

void Oscillator::reset()
{
    increment_ = 0.0f;
    phase_ = 0.0f;
}

void Oscillator::setAmplitude(const float amplitude)
{
    amplitude_ = amplitude;
}

void Oscillator::setPeriod(const float period)
{
    period_ = period;
}

float Oscillator::nextSample()
{
    float output{0.0f};

    phase_ += increment_;

    if (phase_ <= quarterPi)
    {
        const float halfPeriod{period_ / 2.0f};
        phaseMax_ = std::floor(0.5f + halfPeriod) - 0.5f;
        phaseMax_ *= pi;

        increment_ = phaseMax_ / halfPeriod;
        phase_ = -phase_;

        if (phase_ * phase_ > 1e-9)
        {
            output = amplitude_ * (std::sinf(phase_) / phase_);
        }
        else
        {
            output = amplitude_;
        }
    }
    else
    {
        if (phase_ > phaseMax_)
        {
            phase_ = (2 * phaseMax_) - phase_;
            increment_ = -increment_;
        }

        output = amplitude_ * (std::sinf(phase_) / phase_);
    }

    return output;
}

} // namespace synth
