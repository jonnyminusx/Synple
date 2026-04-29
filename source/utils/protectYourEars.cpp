#include "protectYourEars.h"

#include "synth/AudioBuffer.h"

#include <cmath>
#include <iostream>

namespace utils
{

namespace
{

void printWarning([[maybe_unused]] const std::string& problem,
                  [[maybe_unused]] const std::string& solution,
                  [[maybe_unused]] const size_t channel,
                  [[maybe_unused]] const size_t sample)
{
#ifndef NDEBUG
    std::cerr << "Warning: Sample value is " << problem
              << " at channel: " << channel << ", sample: " << sample
              << " - " << solution << "\n";
#endif
}

} // namespace

void protectYourEars(synth::AudioBuffer& buffer)
{
    for (size_t channel = 0; channel < buffer.channelCount(); ++channel)
    {
        bool shouldSilence{false};

        size_t sampleIndex = 0;
        for (float& sampleValue : buffer.channelBuffer(channel))
        {
            if (std::isnan(sampleValue))
            {
                printWarning("nan", "silencing channel", channel, sampleIndex);
                shouldSilence = true;
                break;
            }
            else if (std::isinf(sampleValue))
            {
                printWarning("inf", "silencing channel", channel, sampleIndex);
                shouldSilence = true;
                break;
            }
            else if (sampleValue < -2.0f || sampleValue > 2.0f)
            {
                printWarning("out of range", "silencing channel", channel, sampleIndex);
                shouldSilence = true;
                break;
            }
            else if (sampleValue < -1.0f)
            {
                printWarning("out of range", "clamping to -1.0f", channel, sampleIndex);
                sampleValue = -1.0f;
            }
            else if (sampleValue > 1.0f)
            {
                printWarning("out of range", "clamping to 1.0f", channel, sampleIndex);
                sampleValue = 1.0f;
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