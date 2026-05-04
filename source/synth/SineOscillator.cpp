#include "SineOscillator.h"

#include "utils/constants.h"
#include <cmath>

namespace synth
{

void SineOscillator::reset()
{
    phase_ = 0.0f;

    sinN_ = amplitude_ * std::sin(constants::tau * phase_);
    // sinNm1_ represents sin(phase - normalizedFreq_), i.e. one step back.
    // Using -normalizedFreq_ here so the first nextSample() returns sin(+freq),
    // giving a sine that rises from zero rather than one that dips negative.
    sinNm1_ = amplitude_ * std::sin(-constants::tau * normalizedFreq_);
    sinRecurrenceCoeff_ = 2.0f * std::cos(normalizedFreq_ * constants::tau);
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
    const float sinx = (sinRecurrenceCoeff_ * sinN_) - sinNm1_;
    sinNm1_ = sinN_;
    sinN_ = sinx;
    return sinx;
}

} // namespace synth
