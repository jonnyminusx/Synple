#include "Voice.h"

#include "../utils/constants.h"
#include "GlideMode.h"
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

int calculateNoteDistance(const int note, const int lastNote, const GlideMode glideMode, const bool isLegato)
{
    int noteDistance{0};

    if (lastNote)
    {
        if (glideMode == GlideMode::Always || (glideMode == GlideMode::Legato && isLegato))
        {
            noteDistance = note - lastNote;
        }
    }

    return noteDistance;
}

} // namespace

void Voice::reset()
{
    note_ = 0;
    saw_ = 0.0f;
    oscillator1_.reset();
    oscillator2_.reset();
    envelope_.reset();
    filter_.reset();
    panLeft_ = 0.707f;
    panRight_ = 0.707f;
}

void Voice::noteOn(const int note,
                   const int lastNote,
                   const int velocity,
                   const float velocitySensitivity,
                   const float volumeTrim,
                   const float oscillatorMix,
                   const float tune,
                   const float detune,
                   const float glideBend,
                   const float sampleRate,
                   const size_t voiceIdx,
                   const bool pwm,
                   const bool isPlayingLegatoStyle,
                   const GlideMode glideMode)
{
    const float adjustedVelocity{(0.004f * static_cast<float>((velocity + 64) * (velocity + 64))) - 8.0f};
    const float osciillator1Amplitude{volumeTrim * adjustedVelocity};
    const float period{calculatePeriod(note, tune, detune, voiceIdx)};
    const int noteDistance{calculateNoteDistance(note, lastNote, glideMode, isPlayingLegatoStyle)};

    note_ = note;
    cutoff_ = sampleRate / (period * constants::pi);
    cutoff_ *= std::exp(velocitySensitivity * static_cast<float>(velocity - 64));
    targetPeriod_ = period;
    period_ = period * std::pow(1.059463094359f, static_cast<float>(noteDistance) - glideBend);

    if (period_ < 6.0f)
    {
        period_ = 6.0f;
    }

    oscillator1_.setAmplitude(osciillator1Amplitude);
    oscillator2_.setAmplitude(osciillator1Amplitude * oscillatorMix);

    if (pwm)
    {
        oscillator2_.squareWave(oscillator1_, period_);
    }

    updatePanning();
}

void Voice::noteOnRestart(const int note,
                          const int velocity,
                          const float velocitySensitivity,
                          const float tune,
                          const float detune,
                          const float sampleRate,
                          const size_t voiceIdx,
                          const GlideMode glideMode)
{
    const float period{calculatePeriod(note, tune, detune, voiceIdx)};

    note_ = note;
    cutoff_ = sampleRate / (period * constants::pi);
    cutoff_ *= std::exp(velocitySensitivity * static_cast<float>(velocity - 64));

    targetPeriod_ = period;

    if (GlideMode::Off == glideMode)
    {
        period_ = period;
    }

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

void Voice::updateLfo(const float glideRate, const float filterMod)
{
    period_ += glideRate * (targetPeriod_ - period_);

    float modulatedCutoff{cutoff_ * std::exp(filterMod)};
    modulatedCutoff = std::clamp(modulatedCutoff, 30.0f, 20000.0f);
    filter_.updateCoefficients(modulatedCutoff, 0.707f);
}

void Voice::updatePeriod(const float pitchBend, const float detune)
{
    oscillator1_.setPeriod(period_ * pitchBend);
    oscillator2_.setPeriod(period_ * detune * pitchBend);
}

void Voice::setModulation(const float modulationOsc1, const float modulationOsc2)
{
    if (envelope_.isActive())
    {
        oscillator1_.setModulation(modulationOsc1);
        oscillator2_.setModulation(modulationOsc2);
    }
}

Output Voice::render(const float input, const float pitchBend, const float detune)
{
    Output output;

    updatePeriod(pitchBend, detune);

    if (!envelope_.isActive())
    {
        envelope_.reset();
        filter_.reset();
        return output;
    }

    const float sample1{oscillator1_.nextSample()};
    const float sample2{oscillator2_.nextSample()};

    saw_ = saw_ * 0.997f + sample1 - sample2;

    output.left = (saw_ + input);
    output.right = output.left;
    output.filter(filter_);
    output *= envelope_.nextValue();

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

const Filter& Voice::filter() const
{
    return filter_;
}

Filter& Voice::filter()
{
    return filter_;
}

} // namespace synth
