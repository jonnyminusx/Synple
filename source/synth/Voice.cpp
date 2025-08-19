#include "Voice.h"

namespace synth
{

namespace
{

float calculatePeriod(const int note, const float tune, const float detune)
{
    float period{tune * std::exp(-0.05776226505f * float(note))};

    while (period < 6.0f || (period * detune) < 6.0f)
    {
        period += period;
    }

    return period;
}

} // namespace

void Voice::reset()
{
    note_ = 0;
    saw_ = 0.0f;
    oscillator1_.reset();
    oscillator2_.reset();
    envelope_.reset();
    pitchBend_ = 1.0f;
}

void Voice::noteOn(const int note, const int velocity)
{
    const float osciillator1Amplitude{0.5f * (static_cast<float>(velocity) / 127.0f)};

    note_ = note;
    period_ = calculatePeriod(note, tune_, oscillatorDetune_);
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
    const float period{period_ * pitchBend_};
    oscillator1_.setPeriod(period);
    oscillator2_.setPeriod(period * oscillatorDetune_);

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

void Voice::setPitchBend(const float pitchBend)
{
    pitchBend_ = pitchBend;
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
