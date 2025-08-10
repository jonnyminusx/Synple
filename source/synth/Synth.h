#pragma once

#include "NoiseGenerator.h"
#include "Voice.h"

#include <cstdint>

namespace synth
{

class AudioBuffer;

class Synth
{
  public:
    void allocateResources(const float sampleRate, const int samplesPerBlock);
    void deallocateResources() const;
    void reset();
    void render(AudioBuffer& audioBuffer);
    void midiMessage(const uint8_t data0, const uint8_t data1, const uint8_t data2);
    void setNoiseMix(const float noiseMix);
    void setEnvelopeDecay(const float decayTime);

  private:
    void noteOn(const int note, const int velocity);
    void noteOff(const int note);

    Voice voice_;

    NoiseGenerator noiseGenerator_;
    float noiseMix_{0.0f};
};

} // namespace synth
