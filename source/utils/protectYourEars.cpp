#include "protectYourEars.h"

#include <juce_core/juce_core.h>

#include <cmath>

namespace utils
{

namespace
{
void printWarning([[maybe_unused]] const std::string& problem,
                  [[maybe_unused]] const std::string& solution,
                  [[maybe_unused]] const int channel,
                  [[maybe_unused]] const int sample)
{
    DBG("Warning: Sample value is " << problem << " at channel: " << channel << ", sample: " << sample << " - "
                                    << solution);
}

} // namespace

void protectYourEars(synth::AudioBuffer& buffer)
{
    for (int channel = 0; channel < buffer.channelCount(); ++channel)
    {
        bool shouldSilence{false};

        for (int sample = 0; sample < buffer.sampleCount(); ++sample)
        {
            float& sampleValue = buffer.sample(channel, sample);

            if (std::isnan(sampleValue))
            {
                printWarning("nan", "silencing channel", channel, sample);
                shouldSilence = true;
                break;
            }
            else if (std::isinf(sampleValue))
            {
                printWarning("inf", "silencing channel", channel, sample);
                shouldSilence = true;
                break;
            }
            else if (sampleValue < -2.0f || sampleValue > 2.0f)
            {
                printWarning("out of range", "silencing channel", channel, sample);
                shouldSilence = true;
                break;
            }
            else if (sampleValue < -1.0f)
            {
                printWarning("out of range", "clamping to -1.0f", channel, sample);
                sampleValue = -1.0f;
            }
            else if (sampleValue > 1.0f)
            {
                printWarning("out of range", "clamping to 1.0f", channel, sample);
                sampleValue = 1.0f;
            }
        }

        if (shouldSilence)
        {
            buffer.clear(channel);
        }
    }
}

} // namespace utils