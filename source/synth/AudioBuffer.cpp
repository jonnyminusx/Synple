#include "AudioBuffer.h"

#include <algorithm>
#include <cassert>
#include <utility>

namespace synth
{

AudioBuffer::AudioBuffer(std::vector<std::span<float>> channels) : channels_(std::move(channels))
{
    // Assume all channels have the same size
    if (!channels_.empty())
    {
        [[maybe_unused]] const auto expectedSize = channels_[0].size();
        assert(std::all_of(channels_.begin(), channels_.end(), [expectedSize](const auto& span) {
            return span.size() == expectedSize;
        }));
    }
}

std::span<float> AudioBuffer::channelBuffer(const size_t channel)
{
    assert(channel >= 0);
    assert(channel < channelCount());
    return channels_[channel];
}

std::span<const float> AudioBuffer::channelBuffer(const size_t channel) const
{
    assert(channel >= 0);
    assert(channel < channelCount());
    return channels_[channel];
}

size_t AudioBuffer::sampleCount() const
{
    return channels_.empty() ? 0 : channels_[0].size();
}

size_t AudioBuffer::channelCount() const
{
    return channels_.size();
}

void AudioBuffer::clear(const size_t channel)
{
    assert(channel >= 0);
    assert(channel < channelCount());

    std::fill(channels_[channel].begin(), channels_[channel].end(), 0.0f);
}

} // namespace synth
