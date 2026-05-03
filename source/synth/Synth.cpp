#include "Synth.h"

#include "AudioBuffer.h"
#include "Envelope.h"
#include "GlideMode.h"
#include "Output.h"
#include "midi/MidiState.h"
#include "utils/constants.h"
#include "utils/sanitiseBuffer.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <span>

namespace synth
{

Synth::Synth() : midiProcessor_(*this) {}

void Synth::allocateResources(const float sampleRate, [[maybe_unused]] const int samplesPerBlock)
{
    sampleRate_ = sampleRate;

    for (Voice& voice : voices_)
    {
        voice.filter().setSampleRate(sampleRate_);
    }
}

void Synth::deallocateResources() const {}

void Synth::reset()
{
    for (Voice& voice : voices_)
    {
        voice.reset();
    }

    noiseGenerator_.reset();
    midiProcessor_.state() = midi::MidiState{};
    outputLevelSmoother_.reset(sampleRate_, 0.05);
    lfo_ = 0.0f;
    lfoStep_ = 0;
    filterZip_ = 0.0f;
}

void Synth::render(AudioBuffer& audioBuffer)
{
    const auto audioBufferLeft{audioBuffer.channelBuffer(0)};
    const auto audioBufferRight{audioBuffer.channelCount() > 1 ? audioBuffer.channelBuffer(1) : std::span<float>{}};

    for (size_t sampleIndex = 0; sampleIndex < audioBuffer.sampleCount(); ++sampleIndex)
    {
        updateLfo();

        const float noise{noiseGenerator_.nextValue() * noiseMix_};

        Output output;

        for (Voice& voice : voices_)
        {
            output += voice.render(noise, midiProcessor_.state().pitchBend, parameters_.detune);
        }

        output *= outputLevelSmoother_.getNextValue();

        if (audioBuffer.channelCount() > 1)
        {
            audioBufferLeft[sampleIndex] = output.left;
            audioBufferRight[sampleIndex] = output.right;
        }
        else
        {
            audioBufferLeft[sampleIndex] = (output.left + output.right) * 0.5f;
        }
    }

    utils::sanitiseBuffer(audioBuffer);
}

void Synth::noteOn(const int note, const int velocity)
{
    int vel = velocity;
    if (ignoreVelocity_)
    {
        vel = 80;
    }

    const size_t voiceIdx{selectVoiceIndexToUse()};
    if (isPolyphonic())
    {
        startVoice(voiceIdx, note, vel);
    }
    else
    {
        if (voices_[voiceIdx].note() > 0)
        {
            shiftQueuedNotes();
            restartMonoVoice(note, vel);
        }
        else
        {
            startVoice(voiceIdx, note, vel);
        }
    }
}

void Synth::noteOff(const int note)
{
    processLastNotePriority(note);

    for (Voice& voice : voices_)
    {
        voice.noteOff(note, midiProcessor_.state().sustainPedal);
    }
}

void Synth::allNotesOff()
{
    for (Voice& voice : voices_)
    {
        voice.reset();
    }
}

void Synth::sustainPedalReleased()
{
    if (!isPolyphonic() && voices_[0].note() == Voice::sustain)
    {
        const int queued = nextQueuedNote();
        if (queued > 0)
        {
            restartMonoVoice(queued, -1);
            return;
        }
    }

    for (Voice& voice : voices_)
    {
        voice.noteOff(Voice::sustain, false);
    }
}

void Synth::setVolumeTrim(const float volumeTrim)
{
    parameters_.volumeTrim = volumeTrim;
}

void Synth::updateLfo()
{
    if (--lfoStep_ < 0)
    {
        lfoStep_ = lfoMaxSamplesPerUpdate_;

        lfo_ += lfoIncrement_;
        if (lfo_ > constants::pi)
        {
            lfo_ -= constants::tau;
        }

        const midi::MidiState& midi{midiProcessor_.state()};

        const float sineValue{std::sin(lfo_)};
        const float vibratoModulation{1.0f + sineValue * (midi.modWheel + vibratoAmount_)};
        const float pwm{1.0f + sineValue * (midi.modWheel + pwmDepth_)};
        const float filterMod{filterKeyTracking_ + midi.filterControl + (filterLfoDepth_ + midi.pressure) * sineValue};
        filterZip_ += 0.005f * (filterMod - filterZip_);

        for (Voice& voice : voices_)
        {
            voice.setModulation(vibratoModulation, pwm);
            voice.updateLfo(glideRate_, filterZip_, filterQ_ * midi.resonanceCtl, midi.pitchBend, filterEnvDepth_);
            voice.updatePeriod(midi.pitchBend, parameters_.detune);
        }
    }
}

void Synth::shiftQueuedNotes()
{
    for (size_t voiceIdx = maxNumVoices_ - 1; voiceIdx > 0; --voiceIdx)
    {
        voices_[voiceIdx].note() = voices_[voiceIdx - 1].note();
        voices_[voiceIdx].release();
    }
}

int Synth::nextQueuedNote()
{
    int note{0};
    for (size_t voiceIdx = 1; voiceIdx < maxNumVoices_; ++voiceIdx)
    {
        const int voiceNote{voices_[voiceIdx].note()};
        if (voiceNote > 0)
        {
            voices_[voiceIdx].note() = 0;
            note = voiceNote;
            break;
        }
    }

    return note;
}

void Synth::startVoice(const size_t voiceIdx, const int note, const int velocity)
{
    if (voiceIdx >= voices_.size())
    {
        return;
    }

    Voice& voice = voices_[voiceIdx];

    Envelope& envelope = voice.envelope();
    envelope.attack();

    Envelope& filterEnvelope = voice.filterEnvelope();
    filterEnvelope.attack();

    voice.noteOn(note, lastNote_, velocity, sampleRate_, voiceIdx, isInPwmMode(), isPlayingLegatoStyle(), parameters_);
    lastNote_ = note;
}

void Synth::restartMonoVoice(const int note, [[maybe_unused]] const int velocity)
{
    voices_[0].noteOnRestart(note, velocity, sampleRate_, 0, parameters_);
}

size_t Synth::selectVoiceIndexToUse() const
{
    if (!isPolyphonic())
    {
        return 0;
    }

    size_t idx{0};
    float minLevel{std::numeric_limits<float>::max()};

    for (size_t i = 1, n = voices_.size(); i < n; ++i)
    {
        const Voice& voice{voices_[i]};

        if (!voice.envelope().isInAttack())
        {
            const float currentLevel{voice.envelope().currentValue()};
            if (currentLevel < minLevel)
            {
                minLevel = currentLevel;
                idx = i;
            }
        }
    }

    return idx;
}

bool Synth::isPolyphonic() const
{
    return numVoices_ > 1;
}

bool Synth::isInPwmMode() const
{
    return vibratoAmount_ == 0.0f && pwmDepth_ > 0.0f;
}

bool Synth::isPlayingLegatoStyle() const
{
    return std::any_of(voices_.begin(), voices_.end(), [](const Voice& voice) { return voice.note() > 0; });
}

void Synth::processLastNotePriority(const int note)
{
    if (isPolyphonic())
    {
        return;
    }

    if (voices_[0].note() != note)
    {
        return;
    }

    int queuedNote{nextQueuedNote()};

    if (queuedNote > 0)
    {
        restartMonoVoice(queuedNote, -1);
    }
}

void Synth::setNoiseMix(const float noiseMix)
{
    noiseMix_ = noiseMix;
}

void Synth::setOscillatorMix(const float oscillatorMix)
{
    parameters_.oscillatorMix = oscillatorMix;
}

void Synth::setDetune(const float detune)
{
    parameters_.detune = detune;
}

void Synth::setTune(const float tune)
{
    parameters_.tune = tune;
}

void Synth::setPolyphonic(const bool polyphonic)
{
    numVoices_ = polyphonic ? maxNumVoices_ : 1;
}

void Synth::setOutputLevel(const float outputLevel)
{
    outputLevelSmoother_.setTargetValue(outputLevel);
}

void Synth::setIgnoreVelocity(const bool ignoreVelocity)
{
    ignoreVelocity_ = ignoreVelocity;
}

void Synth::setVelocitySensitivity(const float velocitySensitivity)
{
    parameters_.velocitySensitivity = velocitySensitivity;
}

void Synth::setLfoIncrement(const float lfoIncrement)
{
    lfoIncrement_ = lfoIncrement;
}

void Synth::setVibratoAmount(const float vibratoAmount)
{
    vibratoAmount_ = vibratoAmount;
}

void Synth::setPwmDepth(const float pwmDepth)
{
    pwmDepth_ = pwmDepth;
}

void Synth::setGlideMode(const int glideMode)
{
    parameters_.glideMode = static_cast<GlideMode>(glideMode);
}

void Synth::setGlideRate(const float glideRate)
{
    glideRate_ = glideRate;
}

void Synth::setGlideBend(const float glideBend)
{
    parameters_.glideBend = glideBend;
}

void Synth::setFilterKeyTracking(const float filterKeyTracking)
{
    filterKeyTracking_ = filterKeyTracking;
}

void Synth::setFilterQ(const float filterQ)
{
    filterQ_ = filterQ;
}

void Synth::setFilterLfoDepth(const float filterLfoDepth)
{
    filterLfoDepth_ = filterLfoDepth;
}

void Synth::setFilterEnvelope(const ADSR& adsr)
{
    for (Voice& voice : voices_)
    {
        voice.filterEnvelope().setADSR(adsr);
    }
}

void Synth::setFilterEnvelopeDepth(const float envDepth)
{
    filterEnvDepth_ = envDepth;
}

void Synth::setOutputLevelInstantly(const float outputLevel)
{
    outputLevelSmoother_.setCurrentAndTargetValue(outputLevel);
}

void Synth::setEnvelope(const ADSR& adsr)
{
    for (Voice& voice : voices_)
    {
        voice.envelope().setADSR(adsr);
    }
}

} // namespace synth
