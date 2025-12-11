#include "Oscillator.h"

#include "../utils/constants.h"
#include <cmath>

namespace synth
{

void Oscillator::reset()
{
    increment_ = 0.0f;
    phase_ = 0.0f;
    dc_ = 0.0f;
}

void Oscillator::setAmplitude(const float amplitude)
{
    amplitude_ = amplitude;
}

void Oscillator::setPeriod(const float period)
{
    period_ = period;
}

void Oscillator::setModulation(const float modulation)
{
    modulation_ = modulation;
}

void Oscillator::squareWave(const Oscillator& other, const float newPeriod)
{
    reset();

    if (other.increment_ > 0.0f)
    {
        phase_ = other.phaseMax_ + other.phaseMax_ - other.phase_;
        increment_ = -other.increment_;
    }
    else if (other.increment_ < 0.0f)
    {
        phase_ = other.phase_;
        increment_ = other.increment_;
    }
    else
    {
        phase_ = -constants::pi;
        increment_ = constants::pi;
    }

    phase_ += constants::pi * newPeriod / 2.0f;
    phaseMax_ = phase_;
}

float Oscillator::nextSample()
{
    float output{0.0f};

    phase_ += increment_;

    if (phase_ <= constants::quarterPi)
    {
        const float halfPeriod{(period_ / 2.0f) * modulation_};
        phaseMax_ = std::floor(0.5f + halfPeriod) - 0.5f;
        dc_ = 0.5f * amplitude_ / phaseMax_;
        phaseMax_ *= constants::pi;

        increment_ = phaseMax_ / halfPeriod;
        phase_ = -phase_;

        sin0_ = amplitude_ * std::sinf(phase_);
        sin1_ = amplitude_ * std::sinf(phase_ - increment_);
        dsin_ = 2.0f * std::cosf(increment_);

        if (phase_ * phase_ > 1e-9)
        {
            output = sin0_ / phase_;
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

        const float sinp{(dsin_ * sin0_) - sin1_};
        sin1_ = sin0_;
        sin0_ = sinp;

        output = sin0_ / phase_;
    }

    return output - dc_;
}

} // namespace synth
