#pragma once

namespace synth
{

class IAudioBuffer
{
  public:
    virtual ~IAudioBuffer() = default;

    virtual float& sample(const int channel, const int sample) = 0;
    virtual float sample(const int channel, const int sample) const = 0;
    virtual int sampleCount() const = 0;
    virtual int channelCount() const = 0;
};

} // namespace synth
