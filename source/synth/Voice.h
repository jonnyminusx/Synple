#pragma once

#include "Envelope.h"
#include "Oscillator.h"
#include <optional>

namespace synth
{

struct Output;
enum class GlideMode;

class Voice
{
  public:
    const static int sustain{-1};

    void reset();
    void noteOn(const int note,
                const int lastNote,
                const int velocity,
                const float volumeTrim,
                const float oscillatorMix,
                const float tune,
                const float detune,
                const float glideBend,
                const size_t voiceIdx,
                const bool pwm,
                const bool isPlayingLegatoStyle,
                const GlideMode glideMode);
    void noteOnRestart(
        const int note, const float tune, const float detune, const size_t voiceIdx, const GlideMode glideMode);
    void noteOff(const int note, const bool sustainPedalPressed);
    void release();
    void updatePanning();
    void updateLfo(const float glideRate);
    void updatePeriod(const float pitchBend, const float detune);
    void setModulation(const float modulationOsc1, const float modulationOsc2);
    Output render(const float input, const float pitchBend, const float detune);

    int note() const;
    int& note();
    const Envelope& envelope() const;
    Envelope& envelope();

  private:
    int note_{0};

    float saw_{0.0f};
    float period_{0.0f};
    float targetPeriod_{0.0f};
    float panLeft_{0.0f};
    float panRight_{0.0f};

    Oscillator oscillator1_;
    Oscillator oscillator2_;
    Envelope envelope_;
};

} // namespace synth
