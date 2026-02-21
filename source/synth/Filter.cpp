#include "Filter.h"
#include "../utils/constants.h"
#include <cmath>

namespace synth
{

void Filter::updateCoefficients(const float cutoff, const float Q)
{
    g_ = std::tan(constants::pi * cutoff / sampleRate_);
    k_ = 1.0f / Q;
    a1_ = 1.0f / (1.0f + g_ * (g_ + k_));
    a2_ = g_ * a1_;
    a3_ = g_ * a2_;
}

void Filter::reset()
{
    g_ = 0.0f;
    k_ = 0.0f;
    a1_ = 0.0f;
    a2_ = 0.0f;
    a3_ = 0.0f;
    ic1eq_ = 0.0f;
    ic2eq_ = 0.0f;
}

float Filter::render(const float x)
{
    const float v3 = x - ic2eq_;
    const float v1 = a1_ * ic1eq_ + a2_ * v3;
    const float v2 = ic2eq_ + a2_ * ic1eq_ + a3_ * v3;
    ic1eq_ = 2.0f * v1 - ic1eq_;
    ic2eq_ = 2.0f * v2 - ic2eq_;
    return v2;
}

void Filter::setSampleRate(const float sampleRate)
{
    sampleRate_ = sampleRate;
}

} // namespace synth
