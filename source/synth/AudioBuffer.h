#pragma once

#include <span>
#include <vector>

namespace synth
{

class AudioBuffer
{
  public:
    AudioBuffer(std::vector<std::span<float>> channels);

    float& sample(const int channel, const int sample);
    float sample(const int channel, const int sample) const;
    int sampleCount() const;
    int channelCount() const;
    void clear(const int channel);

  private:
    std::vector<std::span<float>> channels_;
};

} // namespace synth
