#include "Voice.h"

namespace synth
{

void Voice::reset()
{
    note_ = std::nullopt;
    oscillator_.reset();
}

void Voice::noteOn(const int note, const int velocity)
{
    const float frequency{440.0f * std::pow(2.0f, (note - 69) / 12.0f)};

    note_ = note;
    oscillator_.setAmplitude(0.5f * (static_cast<float>(velocity) / 127.0f));
    oscillator_.setIncrement(frequency / sampleRate_);
    oscillator_.setFrequency(frequency);
    oscillator_.setSampleRate(sampleRate_);
    oscillator_.reset();
}

void Voice::noteOff(const int note)
{
    if (note_ == note)
    {
        note_ = std::nullopt;
    }
}

std::optional<int> Voice::note() const
{
    return note_;
}

float Voice::render()
{
    return note_.has_value() ? oscillator_.nextSample() : 0.0f;
}

void Voice::setSampleRate(const float sampleRate)
{
    sampleRate_ = sampleRate;
}

} // namespace synth
