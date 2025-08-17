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
    void setOscillatorMix(const float oscillatorMix);
    void setOscillatorDetune(const float semi, const float cent);
    void setTune(const float tune);

    void setEnvelopeDecay(const float decayTime);
    void setEnvelopeAttack(const float attackTime);
    void setEnvelopeSustain(const float sustainLevel);
    void setEnvelopeRelease(const float releaseTime);

  private:
    void noteOn(const int note, const int velocity);
    void noteOff(const int note);

    Voice voice_;

    NoiseGenerator noiseGenerator_;
    float noiseMix_{0.0f};

    float envelopeAttack_{0.0f};
    float envelopeDecay_{0.0f};
    float envelopeSustain_{1.0f};
    float envelopeRelease_{0.0f};
};

} // namespace synth
