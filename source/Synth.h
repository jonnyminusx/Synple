#pragma once

#include "Voice.h"
#include <cstdint>

class Synth
{
  public:
    Synth();

    void allocateResources(const double sampleRate, const int samplesPerBlock);
    void deallocateResources() const;
    void reset();
    void render(float **outputBuffers, int sampleCount) const;
    void midiMessage(const uint8_t data0, const uint8_t data1, const uint8_t data2);

  private:
    void noteOn(const int note, const int velocity);
    void noteOff(const int note);

    float sampleRate_{0.0f};
    Voice voice_;
};
