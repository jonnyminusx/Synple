#include "AudioBuffer.h"

AudioBuffer::AudioBuffer(juce::AudioBuffer<float>& buffer, const int startSampleIndex, const int endSampleIndex)
    : buffer_(buffer),
      startSampleIndex_(startSampleIndex),
      endSampleIndex_(endSampleIndex)
{
    jassert(startSampleIndex >= 0);
    jassert(endSampleIndex >= startSampleIndex);
    jassert(endSampleIndex <= buffer_.getNumSamples());
}

float& AudioBuffer::sample(const int channel, const int sample)
{
    jassert(channel >= 0);
    jassert(channel < channelCount());
    jassert(sample >= 0);
    jassert(sample < sampleCount());
    return buffer_.getWritePointer(channel)[sample + startSampleIndex_];
}

float AudioBuffer::sample(const int channel, const int sample) const
{
    jassert(channel >= 0);
    jassert(channel < channelCount());
    jassert(sample >= 0);
    jassert(sample < sampleCount());
    return buffer_.getReadPointer(channel)[sample + startSampleIndex_];
}

int AudioBuffer::sampleCount() const
{
    return endSampleIndex_ - startSampleIndex_;
}

int AudioBuffer::channelCount() const
{
    return buffer_.getNumChannels();
}
