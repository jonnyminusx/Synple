#pragma once

#include <cmath>
#include <numbers>
#include <vector>

namespace dsp
{

// Evaluates the energy at a specific frequency using the Goertzel algorithm.
// A Hann window is applied before evaluation to suppress sidelobes, which
// prevents energy from nearby harmonics leaking into alias frequency bins.
// Returns a raw magnitude comparable across frequencies on the same signal.
inline float goertzel(const std::vector<float>& samples, float targetFreq, float sampleRate)
{
    const int N = static_cast<int>(samples.size());
    const float omega = 2.0f * std::numbers::pi_v<float> * targetFreq / sampleRate;
    const float coeff = 2.0f * std::cos(omega);
    float s1 = 0.0f;
    float s2 = 0.0f;

    for (int i = 0; i < N; ++i)
    {
        const float w = 0.5f - 0.5f * std::cos(2.0f * std::numbers::pi_v<float> * static_cast<float>(i) /
                                               static_cast<float>(N - 1));
        const float s0 = samples[i] * w + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    const float real = s1 - s2 * std::cos(omega);
    const float imag = s2 * std::sin(omega);
    return std::sqrt(real * real + imag * imag);
}

} // namespace dsp
