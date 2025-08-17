#pragma once

#include "Envelope.h"
#include "Oscillator.h"
#include <optional>

namespace synth
{

class Voice
{
  public:
    void reset();
    void noteOn(const int note, const int velocity);
    void noteOff(const int note);
    void release();
    float render(const float input);

    const Envelope& envelope() const;
    Envelope& envelope();

    void setSampleRate(const float sampleRate);
    void setOscillatorMix(const float oscillatorMix);
    void setOscillatorDetune(const float semi, const float cent);
    void setTune(const float tune);

  private:
    int note_{0};

    float sampleRate_{0.0f};
    float saw_{0.0f};
    float oscillatorMix_{0.0f};
    float oscillatorDetune_{0.0f};
    float period_{0.0f};
    float tune_{0.0f};

    Oscillator oscillator1_;
    Oscillator oscillator2_;
    Envelope envelope_;
};

} // namespace synth
