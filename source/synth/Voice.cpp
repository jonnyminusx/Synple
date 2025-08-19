#include "Voice.h"

#include "Output.h"
#include <algorithm>
#include <numbers>

namespace synth
{

namespace
{

constexpr float piOver4{std::numbers::pi_v<float> / 4.0f};

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
    panLeft_ = 0.707f;
    panRight_ = 0.707f;
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

void Voice::updatePanning()
{
    const float panning{std::clamp((note_ - 60) / 24.0f, -1.0f, 1.0f)};
    panLeft_ = std::sin(piOver4 * (1.0f - panning));
    panRight_ = std::sin(piOver4 * (1.0f + panning));
}

Output Voice::render(const float input)
{
    Output output;

    const float period{period_ * pitchBend_};
    oscillator1_.setPeriod(period);
    oscillator2_.setPeriod(period * oscillatorDetune_);

    if (!envelope_.isActive())
    {
        envelope_.reset();
        return output;
    }

    const float sample1{oscillator1_.nextSample()};
    const float sample2{oscillator2_.nextSample()};

    saw_ = saw_ * 0.997f + sample1 - sample2;

    output.left = (saw_ + input) * envelope_.nextValue();
    output.right = output.left;

    output.left *= panLeft_;
    output.right *= panRight_;

    return output;
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
