#include "Synth.h"

Synth::Synth() : sampleRate_(44100.0f)
{
}

void Synth::allocateResources(const double sampleRate, [[maybe_unused]] const int samplesPerBlock)
{
    sampleRate_ = static_cast<float>(sampleRate);
}

void Synth::deallocateResources() const
{
}

void Synth::reset() const
{
}

void Synth::render([[maybe_unused]] float **outputBuffers, [[maybe_unused]] int sampleCount) const
{
}

void Synth::midiMessage([[maybe_unused]] const uint8_t data0, [[maybe_unused]] const uint8_t data1,
                        [[maybe_unused]] const uint8_t data2) const
{
}
