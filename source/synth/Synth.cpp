#include "Synth.h"
#include "../utils/protectYourEars.h"
#include "IAudioBuffer.h"
#include "INoiseGenerator.h"
#include <cassert>

namespace synth
{

Synth::Synth(INoiseGenerator* noiseGenerator) : sampleRate_(44100.0f), noiseGenerator_(noiseGenerator)
{
    assert(noiseGenerator_);
}

void Synth::allocateResources(const double sampleRate, [[maybe_unused]] const int samplesPerBlock)
{
    sampleRate_ = static_cast<float>(sampleRate);
}

void Synth::deallocateResources() const
{
}

void Synth::reset()
{
    voice_.reset();
    noiseGenerator_->reset();
}

void Synth::render(IAudioBuffer& audioBuffer) const
{
    for (int sampleIndex = 0; sampleIndex < audioBuffer.sampleCount(); ++sampleIndex)
    {
        const float noise{noiseGenerator_->nextValue()};
        const float output{voice_.note().has_value() ? noise * voice_.velocityNormalised() * 0.5f : 0.0f};

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
    voice_.noteOn(note, velocity);
}

void Synth::noteOff(const int note)
{
    voice_.noteOff(note);
}

} // namespace synth
