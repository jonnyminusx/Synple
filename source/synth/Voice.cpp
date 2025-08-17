#include "Voice.h"

namespace synth
{

void Voice::reset()
{
    note_ = 0;
    saw_ = 0.0f;
    oscillator1_.reset();
    oscillator2_.reset();
    envelope_.reset();
}

void Voice::noteOn(const int note, const int velocity)
{
    const float frequency{440.0f * std::pow(2.0f, ((note - 69) + tune_) / 12.0f)};
    const float osciillator1Amplitude{0.5f * (static_cast<float>(velocity) / 127.0f)};

    note_ = note;
    period_ = sampleRate_ / frequency;
    oscillator1_.setAmplitude(osciillator1Amplitude);
    oscillator2_.setAmplitude(osciillator1Amplitude * oscillatorMix_);
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
    oscillator1_.setPeriod(period_);
    oscillator2_.setPeriod(period_ * oscillatorDetune_);

    if (!envelope_.isActive())
    {
        envelope_.reset();
        return 0.0f;
    }

    const float sample1{oscillator1_.nextSample()};
    const float sample2{oscillator2_.nextSample()};

    saw_ = saw_ * 0.997f + sample1 - sample2;

    const float output{saw_ + input};
    const float envelopeValue{envelope_.nextValue()};

    return output * envelopeValue;
}

void Voice::setSampleRate(const float sampleRate)
{
    sampleRate_ = sampleRate;
}

void Voice::setOscillatorMix(const float oscillatorMix)
{
    oscillatorMix_ = oscillatorMix;
}

void Voice::setOscillatorDetune(const float semi, const float cent)
{
    oscillatorDetune_ = std::pow(1.059463094359f, -semi - 0.01f * cent);
}

void Voice::setTune(const float tune)
{
    tune_ = tune;
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
