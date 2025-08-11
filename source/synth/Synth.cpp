#include "Synth.h"
#include "../utils/protectYourEars.h"

namespace synth
{

void Synth::allocateResources(const float sampleRate, [[maybe_unused]] const int samplesPerBlock)
{
    voice_.setSampleRate(sampleRate);
}

void Synth::deallocateResources() const
{
}

void Synth::reset()
{
    voice_.reset();
    noiseGenerator_.reset();
}

void Synth::render(AudioBuffer& audioBuffer)
{
    for (int sampleIndex = 0; sampleIndex < audioBuffer.sampleCount(); ++sampleIndex)
    {
        const float noise{noiseGenerator_.nextValue() * noiseMix_};
        const float output{voice_.render(noise)};

        audioBuffer.sample(0, sampleIndex) = output;
        if (audioBuffer.channelCount() > 1)
        {
            audioBuffer.sample(1, sampleIndex) = output;
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
        break;
    case 0xC0: // Program Change
        break;
    case 0xD0: // Channel Aftertouch
        break;
    case 0xE0: // Pitch Bend
        break;
    default:
        break;
    }
}

void Synth::noteOn(const int note, const int velocity)
{
    Envelope& envelope = voice_.envelope();
    envelope.setAttackMultiplier(envelopeAttack_);
    envelope.setDecayMultiplier(envelopeDecay_);
    envelope.setSustainLevel(envelopeSustain_);
    envelope.setReleaseMultiplier(envelopeRelease_);
    envelope.setLevel(1.0f);
    envelope.setTarget(envelopeSustain_);
    envelope.setMultiplier(envelopeDecay_);

    voice_.noteOn(note, velocity);
}

void Synth::noteOff(const int note)
{
    voice_.noteOff(note);
}

void Synth::setNoiseMix(const float noiseMix)
{
    noiseMix_ = noiseMix;
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
