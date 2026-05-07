#pragma once

#include "dsp/Filter.h"

#include "Envelope.h"
#include "SawtoothOscillator.h"
#include "SineOscillator.h"
#include "WaveformType.h"

#include <cstddef>

namespace synth
{

struct Output;
struct Parameters;

class Voice
{
  public:
    const static int sustain{-1};

    void reset();
    void setWaveform(WaveformType waveform);
    void noteOn(const int note,
                const int lastNote,
                const int velocity,
                const float sampleRate,
                const size_t voiceIdx,
                const bool pwm,
                const bool isPlayingLegatoStyle,
                const Parameters& parameters);
    void noteOnRestart(const int note,
                       const int velocity,
                       const float sampleRate,
                       const size_t voiceIdx,
                       const Parameters& parameters);
    void noteOff(const int note, const bool sustainPedalPressed);
    void release();
    void updatePanning();
    void updateLfo(const float glideRate,
                   const float filterMod,
                   const float filterQ,
                   const float pitchBend,
                   const float filterEnvDepth);
    void updatePeriod(const float pitchBend, const float detune);
    void setModulation(const float modulationOsc1, const float modulationOsc2);
    Output render(const float input, const float pitchBend, const float detune);

    int note() const;
    int& note();
    const Envelope& envelope() const;
    Envelope& envelope();

    const Envelope& filterEnvelope() const;
    Envelope& filterEnvelope();

    const dsp::Filter& filter() const;
    dsp::Filter& filter();

  private:
    int note_{0};

    float saw_{0.0f};
    float period_{0.0f};
    float targetPeriod_{0.0f};
    float panLeft_{0.0f};
    float panRight_{0.0f};
    float cutoff_{0.0f};

    WaveformType waveform_{WaveformType::Sawtooth};

    SawtoothOscillator sawOsc1_;
    SawtoothOscillator sawOsc2_;
    SineOscillator sineOsc1_;
    SineOscillator sineOsc2_;

    // Non-owning pointers into the concrete members above; re-seated by setWaveform().
    Oscillator* oscillator1_{&sawOsc1_};
    Oscillator* oscillator2_{&sawOsc2_};

    Envelope envelope_;
    Envelope filterEnvelope_;
    dsp::Filter filter_;
};

} // namespace synth
