#include "Voice.h"

namespace synth
{

void Voice::reset()
{
    note_ = std::nullopt;
    saw_ = 0.0f;
    oscillator_.reset();
}

void Voice::noteOn(const int note, const int velocity)
{
    const float frequency{440.0f * std::pow(2.0f, (note - 69) / 12.0f)};

    note_ = note;
    oscillator_.setAmplitude(0.5f * (static_cast<float>(velocity) / 127.0f));
    oscillator_.setPeriod(sampleRate_ / frequency);
    oscillator_.reset();
    envelope_.setLevel(1.0f);
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
    if (!note_.has_value())
    {
        return 0.0f;
    }

    saw_ = saw_ * 0.997f + oscillator_.nextSample();
    const float envelopeValue{envelope_.nextValue()};

    return saw_ * envelopeValue;
}

void Voice::setSampleRate(const float sampleRate)
{
    sampleRate_ = sampleRate;
}

} // namespace synth
