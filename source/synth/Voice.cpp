#include "Voice.h"

#include "dsp/Filter.h"

#include "Envelope.h"
#include "GlideMode.h"
#include "Output.h"
#include "Parameters.h"
#include "math/constants.h"

#include <algorithm>
#include <cmath>

namespace synth
{

namespace
{

constexpr float kAnalog{0.002f};

float calculatePeriod(const int note, const float tune, const float detune, const size_t voiceIdx)
{
    float period{tune *
                 std::exp(-0.05776226505f * (static_cast<float>(note) + kAnalog * static_cast<float>(voiceIdx)))};

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
    for (auto& osc : oscillators_)
        osc->reset();
    envelope_.reset();
    filterEnvelope_.reset();
    filter_.reset();
    panLeft_ = 0.707f;
    panRight_ = 0.707f;
}

void Voice::setWaveform(const WaveformType waveform)
{
    if (waveform_ == waveform)
        return;

    waveform_ = waveform;

    if (waveform == WaveformType::Sawtooth)
    {
        oscillators_[0] = std::make_unique<SawtoothOscillator>();
        oscillators_[1] = std::make_unique<SawtoothOscillator>();
    }
    else
    {
        oscillators_[0] = std::make_unique<SineOscillator>();
        oscillators_[1] = std::make_unique<SineOscillator>();
    }
}

void Voice::noteOn(const int note,
                   const int lastNote,
                   const int velocity,
                   const float sampleRate,
                   const size_t voiceIdx,
                   const bool pwm,
                   const bool isPlayingLegatoStyle,
                   const Parameters& parameters)
{
    const float adjustedVelocity{(0.004f * static_cast<float>((velocity + 64) * (velocity + 64))) - 8.0f};
    const float oscillator1Amplitude{parameters.output.volumeTrim * adjustedVelocity};
    const float period{calculatePeriod(note, parameters.oscillator.tune, parameters.oscillator.detune, voiceIdx)};
    const int noteDistance{calculateNoteDistance(note, lastNote, parameters.glide.mode, isPlayingLegatoStyle)};

    note_ = note;
    cutoff_ = sampleRate / (period * math::pi);
    cutoff_ *= std::exp(parameters.filter.velocitySensitivity * static_cast<float>(velocity - 64));
    targetPeriod_ = period;
    period_ = period * std::pow(1.059463094359f, static_cast<float>(noteDistance) - parameters.glide.bendSemitones);

    if (period_ < 6.0f)
    {
        period_ = 6.0f;
    }

    oscillators_[0]->setAmplitude(oscillator1Amplitude);

    if (pwm)
    {
        oscillators_[0]->setSquareWave(oscillator1Amplitude * parameters.oscillator.mix, period_);
        oscillators_[1]->setAmplitude(0.0f);
    }
    else
    {
        oscillators_[0]->setSquareWave(0.0f, period_);
        oscillators_[1]->setAmplitude(oscillator1Amplitude * parameters.oscillator.mix);
    }

    oscillators_[0]->noteOn(period_);
    oscillators_[1]->noteOn(period_ * parameters.oscillator.detune);

    updatePanning();
}

void Voice::noteOnRestart(
    const int note, const int velocity, const float sampleRate, const size_t voiceIdx, const Parameters& parameters)
{
    const float period{calculatePeriod(note, parameters.oscillator.tune, parameters.oscillator.detune, voiceIdx)};

    note_ = note;
    cutoff_ = sampleRate / (period * math::pi);
    cutoff_ *= std::exp(parameters.filter.velocitySensitivity * static_cast<float>(velocity - 64));

    targetPeriod_ = period;

    if (GlideMode::Off == parameters.glide.mode)
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
    filterEnvelope_.release();
}

void Voice::updatePanning()
{
    const float panning{std::clamp(static_cast<float>(note_ - 60) / 24.0f, -1.0f, 1.0f)};
    panLeft_ = std::sin(math::quarterPi * (1.0f - panning));
    panRight_ = std::sin(math::quarterPi * (1.0f + panning));
}

void Voice::updateLfo(const float glideRate,
                      const float filterMod,
                      const float filterQ,
                      const float pitchBend,
                      const float filterEnvDepth)
{
    period_ += glideRate * (targetPeriod_ - period_);

    const float filterEnvelopeValue{filterEnvelope_.nextValue() * filterEnvDepth};

    float modulatedCutoff{cutoff_ * std::exp(filterMod + filterEnvelopeValue) / pitchBend};
    modulatedCutoff = std::clamp(modulatedCutoff, 30.0f, 20000.0f);
    filter_.updateCoefficients(modulatedCutoff, filterQ);
}

void Voice::updatePeriod(const float pitchBend, const float detune)
{
    oscillators_[0]->setPeriod(period_ * pitchBend);
    oscillators_[1]->setPeriod(period_ * detune * pitchBend);
}

void Voice::setModulation(const float modulationOsc1, const float modulationOsc2)
{
    if (envelope_.isActive())
    {
        oscillators_[0]->setModulation(modulationOsc1);
        oscillators_[1]->setModulation(modulationOsc2);
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

    const float oscillatorOutput{oscillators_[0]->nextSample() + oscillators_[1]->nextSample()};

    output.left = oscillatorOutput + input;
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

const Envelope& Voice::filterEnvelope() const
{
    return filterEnvelope_;
}

Envelope& Voice::filterEnvelope()
{
    return filterEnvelope_;
}

const dsp::Filter& Voice::filter() const
{
    return filter_;
}

dsp::Filter& Voice::filter()
{
    return filter_;
}

} // namespace synth
