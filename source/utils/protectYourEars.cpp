#include "protectYourEars.h"

#include "synth/AudioBuffer.h"

#include <juce_core/juce_core.h>

#include <cmath>

namespace utils
{

namespace
{
void printWarning(const std::string& problem, const std::string& solution, const size_t channel, const size_t sample)
{
    DBG("Warning: Sample value is " << problem << " at channel: " << channel << ", sample: " << sample << " - "
                                    << solution);
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