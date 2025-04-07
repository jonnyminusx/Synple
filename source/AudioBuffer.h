#pragma once

#include "synth/IAudioBuffer.h"
#include <juce_audio_basics/juce_audio_basics.h>

class AudioBuffer : public synth::IAudioBuffer
{
  public:
    AudioBuffer(juce::AudioBuffer<float>& buffer, const int startSampleIndex, const int endSampleIndex);

    virtual float& sample(const int channel, const int sample) final;
    virtual float sample(const int channel, const int sample) const final;
    virtual int sampleCount() const final;
    virtual int channelCount() const final;
    virtual void clear(const int channel) final;

  private:
    juce::AudioBuffer<float>& buffer_;
    int startSampleIndex_{0};
    int endSampleIndex_{0};
};
