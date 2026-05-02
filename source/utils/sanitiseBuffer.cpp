#include "sanitiseBuffer.h"

#include "synth/AudioBuffer.h"

#include <cmath>
#include <string>
#ifndef NDEBUG
#include <iostream>
#endif

namespace utils
{

namespace
{

constexpr float kClampThreshold = 1.0f;
constexpr float kSilenceThreshold = 2.0f;

#ifndef NDEBUG
void printWarning(const std::string& problem, const std::string& solution, size_t channel, size_t sample)
{
    std::cerr << "Warning: Sample value is " << problem << " at channel: " << channel << ", sample: " << sample << " - "
              << solution << "\n";
}
#else
void printWarning(const std::string&, const std::string&, size_t, size_t)
{
}
#endif

} // namespace

void sanitiseBuffer(synth::AudioBuffer& buffer)
{
    for (size_t channel = 0; channel < buffer.channelCount(); ++channel)
    {
        bool shouldSilence{false};

        size_t sampleIndex = 0;
        for (float& sampleValue : buffer.channelBuffer(channel))
        {
            if (!std::isfinite(sampleValue))
            {
                printWarning("non-finite", "silencing channel", channel, sampleIndex);
                shouldSilence = true;
                break;
            }
            else if (sampleValue < -kSilenceThreshold || sampleValue > kSilenceThreshold)
            {
                printWarning("out of range", "silencing channel", channel, sampleIndex);
                shouldSilence = true;
                break;
            }
            else if (sampleValue < -kClampThreshold)
            {
                printWarning("out of range", "clamping to -1.0f", channel, sampleIndex);
                sampleValue = -kClampThreshold;
            }
            else if (sampleValue > kClampThreshold)
            {
                printWarning("out of range", "clamping to 1.0f", channel, sampleIndex);
                sampleValue = kClampThreshold;
            }
            ++sampleIndex;
        }

        if (shouldSilence)
        {
            buffer.clear(channel);
        }
    }
}

} // namespace utils
