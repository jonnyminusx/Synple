#include "Voice.h"

#include "Output.h"
#include <algorithm>
#include <numbers>

namespace synth
{

namespace
{

constexpr float piOver4{std::numbers::pi_v<float> / 4.0f};
constexpr float analog{0.002f};

float calculatePeriod(const int note, const float tune, const float detune, const size_t voiceIdx)
{
    float period{tune * std::exp(-0.05776226505f * (static_cast<float>(note) + analog * static_cast<float>(voiceIdx)))};

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
    panLeft_ = 0.707f;
    panRight_ = 0.707f;
}

void Voice::noteOn(const int note,
                   const int velocity,
                   const float volumeTrim,
                   const float oscillatorMix,
                   const float tune,
                   const float detune,
                   const size_t voiceIdx)
{
    const float adjustedVelocity{(0.004f * static_cast<float>((velocity + 64) * (velocity + 64))) - 8.0f};
    const float osciillator1Amplitude{volumeTrim * adjustedVelocity};

    note_ = note;
    oscillator1Period_ = calculatePeriod(note, tune, detune, voiceIdx);
    oscillator2Period_ = oscillator1Period_ * detune;
    oscillator1_.setAmplitude(osciillator1Amplitude);
    oscillator2_.setAmplitude(osciillator1Amplitude * oscillatorMix);

    updatePanning();
}

void Voice::noteOnRestart(const int note, const float tune, const float detune, const size_t voiceIdx)
{
    note_ = note;
    oscillator1Period_ = calculatePeriod(note, tune, detune, voiceIdx);
    oscillator2Period_ = oscillator1Period_ * detune;
    envelope_.nudgeLevelUp();
    updatePanning();
}

void Voice::noteOff(const int note, const bool sustainPedalPressed)
{
    if (note_ == note)
    {
        if (sustainPedalPressed)
        {
            note_ = sustain;
        }
        else
        {
            envelope_.release();
            note_ = 0;
        }
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

Output Voice::render(const float input, const float pitchBend)
{
    Output output;

    oscillator1_.setPeriod(oscillator1Period_ * pitchBend);
    oscillator2_.setPeriod(oscillator2Period_ * pitchBend);

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

int Voice::note() const
{
    return note_;
}

int& Voice::note()
{
    return note_;
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
