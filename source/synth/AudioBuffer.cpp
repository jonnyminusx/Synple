#include "AudioBuffer.h"

#include <algorithm>
#include <cassert>

namespace synth
{

AudioBuffer::AudioBuffer(std::vector<std::span<float>> channels) : channels_(std::move(channels))
{
    // Assume all channels have the same size
    if (!channels_.empty())
    {
        const auto expectedSize = channels_[0].size();
        assert(std::all_of(channels_.begin(), channels_.end(), [expectedSize](const auto& span) {
            return span.size() == expectedSize;
        }));
    }
}

float& AudioBuffer::sample(const int channel, const int sample)
{
    assert(channel >= 0);
    assert(channel < channelCount());
    assert(sample >= 0);
    assert(sample < sampleCount());
    return channels_[static_cast<size_t>(channel)][static_cast<size_t>(sample)];
}

float AudioBuffer::sample(const int channel, const int sample) const
{
    assert(channel >= 0);
    assert(channel < channelCount());
    assert(sample >= 0);
    assert(sample < sampleCount());
    return channels_[static_cast<size_t>(channel)][static_cast<size_t>(sample)];
}

int AudioBuffer::sampleCount() const
{
    return channels_.empty() ? 0 : static_cast<int>(channels_[0].size());
}

int AudioBuffer::channelCount() const
{
    return static_cast<int>(channels_.size());
}

void AudioBuffer::clear(const int channel)
{
    assert(channel >= 0);
    assert(channel < channelCount());

    std::fill(channels_[static_cast<size_t>(channel)].begin(), channels_[static_cast<size_t>(channel)].end(), 0.0f);
}

} // namespace synth
