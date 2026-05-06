#include "Oscillator.h"

#include "math/constants.h"
#include <algorithm>
#include <cmath>

namespace synth
{

void Oscillator::reset()
{
    increment_ = 0.0f;
    phase_ = 0.0f;
    dcOffset_ = 0.0f;
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
    // Clamp away from zero: halfPeriod = (period/2) * modulation_ feeds a
    // divisor below; zero or negative values would produce ±inf increment_.
    modulation_ = std::max(modulation, 0.01f);
}

void Oscillator::squareWave(const Oscillator& other, const float newPeriod)
{
    reset();

    if (other.increment_ > 0.0f)
    {
        phase_ = other.halfPhase_ + other.halfPhase_ - other.phase_;
        increment_ = -other.increment_;
    }
    else if (other.increment_ < 0.0f)
    {
        phase_ = other.phase_;
        increment_ = other.increment_;
    }
    else
    {
        phase_ = -math::pi;
        increment_ = math::pi;
    }

    phase_ += math::pi * newPeriod / 2.0f;
    halfPhase_ = phase_;

    // Initialise sine recurrence state so the first nextSample() call
    // produces correct output without a silent half-period transient.
    sinCurrent_ = amplitude_ * std::sin(phase_);
    sinPrevious_ = amplitude_ * std::sin(phase_ - increment_);
    sinRecurrenceCoeff_ = 2.0f * std::cos(increment_);
}

float Oscillator::nextSample()
{
    float output{0.0f};

    phase_ += increment_;

    if (phase_ <= math::quarterPi)
    {
        const float halfPeriod{(period_ / 2.0f) * modulation_};
        halfPhase_ = std::floor(0.5f + halfPeriod) - 0.5f;
        dcOffset_ = 0.5f * amplitude_ / halfPhase_;
        halfPhase_ *= math::pi;

        increment_ = halfPhase_ / halfPeriod;
        phase_ = -phase_;

        sinCurrent_ = amplitude_ * std::sin(phase_);
        sinPrevious_ = amplitude_ * std::sin(phase_ - increment_);
        sinRecurrenceCoeff_ = 2.0f * std::cos(increment_);

        if (phase_ * phase_ > 1e-9)
        {
            output = sinCurrent_ / phase_;
        }
        else
        {
            output = amplitude_;
        }
    }
    else
    {
        if (phase_ > halfPhase_)
        {
            phase_ = (2 * halfPhase_) - phase_;
            increment_ = -increment_;
        }

        const float sinp{(sinRecurrenceCoeff_ * sinCurrent_) - sinPrevious_};
        sinPrevious_ = sinCurrent_;
        sinCurrent_ = sinp;

        output = sinCurrent_ / phase_;
    }

    return output - dcOffset_;
}

} // namespace synth
