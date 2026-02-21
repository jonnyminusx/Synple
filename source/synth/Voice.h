#pragma once

#include "Envelope.h"
#include "Filter.h"
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
                const float velocitySensitivity,
                const float volumeTrim,
                const float oscillatorMix,
                const float tune,
                const float detune,
                const float glideBend,
                const float sampleRate,
                const size_t voiceIdx,
                const bool pwm,
                const bool isPlayingLegatoStyle,
                const GlideMode glideMode);
    void noteOnRestart(const int note,
                       const int velocity,
                       const float velocitySensitivity,
                       const float tune,
                       const float detune,
                       const float sampleRate,
                       const size_t voiceIdx,
                       const GlideMode glideMode);
    void noteOff(const int note, const bool sustainPedalPressed);
    void release();
    void updatePanning();
    void updateLfo(const float glideRate, const float filterMod, const float filterQ);
    void updatePeriod(const float pitchBend, const float detune);
    void setModulation(const float modulationOsc1, const float modulationOsc2);
    Output render(const float input, const float pitchBend, const float detune);

    int note() const;
    int& note();
    const Envelope& envelope() const;
    Envelope& envelope();

    const Filter& filter() const;
    Filter& filter();

  private:
    int note_{0};

    float saw_{0.0f};
    float period_{0.0f};
    float targetPeriod_{0.0f};
    float panLeft_{0.0f};
    float panRight_{0.0f};
    float cutoff_{0.0f};

    Oscillator oscillator1_;
    Oscillator oscillator2_;
    Envelope envelope_;
    Filter filter_;
};

} // namespace synth
