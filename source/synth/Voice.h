#pragma once

#include "Envelope.h"
#include "Oscillator.h"
#include <optional>

namespace synth
{

struct Output;

class Voice
{
  public:
    void reset();
    void noteOn(const int note, const int velocity, const float oscillatorMix, const float tune, const float detune);
    void noteOff(const int note);
    void release();
    void updatePanning();
    Output render(const float input, const float pitchBend);

    const Envelope& envelope() const;
    Envelope& envelope();

    void setTune(const float tune);

  private:
    int note_{0};

    float saw_{0.0f};
    float oscillator1Period_{0.0f};
    float oscillator2Period_{0.0f};
    float panLeft_{0.0f};
    float panRight_{0.0f};

    Oscillator oscillator1_;
    Oscillator oscillator2_;
    Envelope envelope_;
};

} // namespace synth
