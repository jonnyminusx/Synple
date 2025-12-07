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
    const static int sustain{-1};

    void reset();
    void noteOn(const int note,
                const int velocity,
                const float volumeTrim,
                const float oscillatorMix,
                const float tune,
                const float detune,
                const size_t voiceIdx);
    void noteOnRestart(const int note, const float tune, const float detune, const size_t voiceIdx);
    void noteOff(const int note, const bool sustainPedalPressed);
    void release();
    void updatePanning();
    Output render(const float input, const float pitchBend);

    int note() const;
    int& note();
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
