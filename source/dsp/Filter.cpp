#include "Filter.h"

#include "utils/constants.h"

#include <cmath>

// Second-order state variable filter (SVF) using trapezoidal (bilinear) integration.
//
// Reference: Simper, "Solving the Continuous SVF Equations Using Trapezoidal
// Integration and Equivalent Circuits", 2013.
//
// Signal flow (per sample):
//   v3 = x  - s2_                        (highpass tap — discarded)
//   v1 = a1_·s1_ + a2_·v3               (bandpass tap — discarded)
//   v2 = s2_ + a2_·s1_ + a3_·v3        (lowpass tap  — returned)
//   s1' = 2·v1 - s1_
//   s2' = 2·v2 - s2_
//
// Coefficients (computed in updateCoefficients):
//   g       = tan(π·fc/fs)           — prewarp maps analog fc to digital domain
//   damping = 1/Q                    — √2 ≈ 0.707 gives Butterworth alignment
//   a1      = 1 / (1 + g·(g+damping))
//   a2      = g·a1
//   a3      = g·a2

namespace dsp
{

void Filter::updateCoefficients(const float cutoff, const float Q)
{
    g_ = std::tan(constants::pi * cutoff / sampleRate_);
    damping_ = 1.0f / Q;
    a1_ = 1.0f / (1.0f + g_ * (g_ + damping_));
    a2_ = g_ * a1_;
    a3_ = g_ * a2_;
}

void Filter::reset()
{
    s1_ = 0.0f;
    s2_ = 0.0f;
}

float Filter::render(const float x)
{
    const float v3 = x - s2_;
    const float v1 = a1_ * s1_ + a2_ * v3;
    const float v2 = s2_ + a2_ * s1_ + a3_ * v3;
    s1_ = 2.0f * v1 - s1_;
    s2_ = 2.0f * v2 - s2_;
    // Flush denormals: prevents x86 soft-float slowdown during long quiet sustains.
    if (std::abs(s1_) < 1e-30f)
        s1_ = 0.0f;
    if (std::abs(s2_) < 1e-30f)
        s2_ = 0.0f;
    return v2;
}

void Filter::setSampleRate(const float sampleRate)
{
    sampleRate_ = sampleRate;
}

} // namespace dsp
