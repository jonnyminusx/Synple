#pragma once

#include "synth/IAudioBuffer.h"
#include <juce_audio_basics/juce_audio_basics.h>

class AudioBuffer : public synth::IAudioBuffer
{
  public:
    AudioBuffer(juce::AudioBuffer<float> &buffer, const int bufferOffset) : buffer_(buffer), bufferOffset_(bufferOffset)
    {
    }

  private:
    [[maybe_unused]] juce::AudioBuffer<float> &buffer_;
    [[maybe_unused]] int bufferOffset_{0};
};
