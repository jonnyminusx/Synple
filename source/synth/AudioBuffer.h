#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace synth
{

class AudioBuffer
{
  public:
    AudioBuffer(juce::AudioBuffer<float>& buffer, const int startSampleIndex, const int endSampleIndex);

    float& sample(const int channel, const int sample);
    float sample(const int channel, const int sample) const;
    int sampleCount() const;
    int channelCount() const;
    void clear(const int channel);

  private:
    juce::AudioBuffer<float>& buffer_;
    int startSampleIndex_{0};
    int endSampleIndex_{0};
};

} // namespace synth
