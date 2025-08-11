#include "Voice.h"

namespace synth
{

void Voice::reset()
{
    note_ = 0;
    saw_ = 0.0f;
    oscillator_.reset();
    envelope_.reset();
}

void Voice::noteOn(const int note, const int velocity)
{
    const float frequency{440.0f * std::pow(2.0f, (note - 69) / 12.0f)};

    note_ = note;
    oscillator_.setAmplitude(0.5f * (static_cast<float>(velocity) / 127.0f));
    oscillator_.setPeriod(sampleRate_ / frequency);
    oscillator_.reset();
}

void Voice::noteOff(const int note)
{
    if (note_ == note)
    {
        envelope_.release();
    }
}

void Voice::release()
{
    envelope_.release();
}

float Voice::render(const float input)
{
    if (!envelope_.isActive())
    {
        envelope_.reset();
        return 0.0f;
    }

    saw_ = saw_ * 0.997f + oscillator_.nextSample();

    const float output{saw_ + input};
    const float envelopeValue{envelope_.nextValue()};

    return output * envelopeValue;
}

void Voice::setSampleRate(const float sampleRate)
{
    sampleRate_ = sampleRate;
}

const Envelope& Voice::envelope() const
{
    return envelope_;
}

Envelope& Voice::envelope()
{
    return envelope_;
}

} // namespace synth
