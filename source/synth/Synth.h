#pragma once

#include "Voice.h"
#include <cstdint>

namespace synth
{

class INoiseGenerator;
class IAudioBuffer;

class Synth
{
  public:
    Synth(INoiseGenerator *noiseGenerator);

    void allocateResources(const double sampleRate, const int samplesPerBlock);
    void deallocateResources() const;
    void reset();
    void render(IAudioBuffer &audioBuffer, int sampleCount) const;
    void midiMessage(const uint8_t data0, const uint8_t data1, const uint8_t data2);

  private:
    void noteOn(const int note, const int velocity);
    void noteOff(const int note);

    float sampleRate_{0.0f};
    Voice voice_;
    std::unique_ptr<INoiseGenerator> noiseGenerator_;
};

} // namespace synth
