#include "Synth.h"
#include "../utils/protectYourEars.h"
#include "Output.h"

namespace synth
{

void Synth::allocateResources(const float sampleRate, [[maybe_unused]] const int samplesPerBlock)
{
    sampleRate_ = sampleRate;
}

void Synth::deallocateResources() const
{
}

void Synth::reset()
{
    for (Voice& voice : voices_)
    {
        voice.reset();
    }
    noiseGenerator_.reset();
    pitchBend_ = 1.0f;
    sustainPedalPressed_ = false;
    outputLevelSmoother_.reset(sampleRate_, 0.05);
}

void Synth::render(AudioBuffer& audioBuffer)
{
    for (int sampleIndex = 0; sampleIndex < audioBuffer.sampleCount(); ++sampleIndex)
    {
        const float noise{noiseGenerator_.nextValue() * noiseMix_};

        Output output;

        for (Voice& voice : voices_)
        {
            output += voice.render(noise, pitchBend_);
        }

        output *= outputLevelSmoother_.getNextValue();

        if (audioBuffer.channelCount() > 1)
        {
            audioBuffer.sample(0, sampleIndex) = output.left;
            audioBuffer.sample(1, sampleIndex) = output.right;
        }
        else
        {
            audioBuffer.sample(0, sampleIndex) = (output.left + output.right) * 0.5f;
        }
    }

    utils::protectYourEars(audioBuffer);
}

void Synth::midiMessage(const uint8_t data0, const uint8_t data1, const uint8_t data2)
{
    switch (data0 & 0xF0)
    {
    case 0x80: // Note Off
    {
        const uint8_t note = data1 & 0x7F;
        noteOff(note);
        break;
    }
    case 0x90: // Note On
    {
        const uint8_t note = data1 & 0x7F;
        const uint8_t velocity = data2 & 0x7F;
        if (velocity == 0)
        {
            noteOff(note);
        }
        else
        {
            noteOn(note, velocity);
        }
        break;
    }
    case 0xB0: // Control Change
        controlChange(data1 & 0x7F, data2 & 0x7F);
        break;
    case 0xC0: // Program Change
        break;
    case 0xD0: // Channel Aftertouch
        break;
    case 0xE0: // Pitch Bend
        pitchBend_ = std::exp(-0.000014102f * data1 + (128 * data2) - 8192);
        break;
    default:
        break;
    }
}

void Synth::controlChange(const uint8_t controller, const uint8_t value)
{
    switch (controller)
    {
    case 0x40: // Sustain pedal
        sustainPedalPressed_ = value >= 64;

        if (!sustainPedalPressed_)
        {
            noteOff(Voice::sustain);
        }

        break;

    default: // All notes off
        if (controller >= 0x78)
        {
            for (Voice& voice : voices_)
            {
                voice.reset();
            }
            sustainPedalPressed_ = false;
        }

        break;
    }
}

void Synth::updateVolumeTrim()
{
    volumeTrim_ = 0.0008f * (3.2f - oscillatorMix_ - 25.0f * noiseMix_) * 1.5f;
}

void Synth::noteOn(const int note, const int velocity)
{
    const size_t voiceIdx{selectVoiceIndexToUse()};
    if (isPolyphonic())
    {
        startVoice(voiceIdx, note, velocity);
    }
    else
    {
        if (voices_[voiceIdx].note() > 0)
        {
            restartMonoVoice(note);
        }
        else
        {
            startVoice(voiceIdx, note, velocity);
        }
    }
}

void Synth::startVoice(const size_t voiceIdx, const int note, const int velocity)
{
    if (voiceIdx >= voices_.size())
    {
        return;
    }

    Voice& voice = voices_[voiceIdx];

    Envelope& envelope = voice.envelope();
    envelope.setAttackMultiplier(envelopeAttack_);
    envelope.setDecayMultiplier(envelopeDecay_);
    envelope.setSustainLevel(envelopeSustain_);
    envelope.setReleaseMultiplier(envelopeRelease_);
    envelope.attack();

    voice.noteOn(note, velocity, volumeTrim_, oscillatorMix_, tune_, detune_, voiceIdx);
}

void Synth::restartMonoVoice(const int note)
{
    voices_[0].noteOnRestart(note, tune_, detune_, 0);
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

void Synth::noteOff(const int note)
{
    for (Voice& voice : voices_)
    {
        voice.noteOff(note, sustainPedalPressed_);
    }
}

void Synth::setNoiseMix(const float noiseMix)
{
    noiseMix_ = noiseMix;
}

void Synth::setOscillatorMix(const float oscillatorMix)
{
    oscillatorMix_ = oscillatorMix;
}

void Synth::setDetune(const float semi, const float cent)
{
    detune_ = std::pow(1.059463094359f, -semi - 0.01f * cent);
}

void Synth::setTune(const float tune)
{
    tune_ = tune;
}

void Synth::setPolyphonic(const bool polyphonic)
{
    numVoices_ = polyphonic ? maxNumVoices_ : 1;
}

void Synth::setOutputLevel(const float outputLevel)
{
    outputLevelSmoother_.setTargetValue(outputLevel);
}

void Synth::setOutputLevelInstantly(const float outputLevel)
{
    outputLevelSmoother_.setCurrentAndTargetValue(outputLevel);
}

void Synth::setEnvelopeDecay(const float decayTime)
{
    envelopeDecay_ = decayTime;
}

void Synth::setEnvelopeAttack(const float attackTime)
{
    envelopeAttack_ = attackTime;
}

void Synth::setEnvelopeSustain(const float sustainLevel)
{
    envelopeSustain_ = sustainLevel;
}

void Synth::setEnvelopeRelease(const float releaseTime)
{
    envelopeRelease_ = releaseTime;
}

} // namespace synth
