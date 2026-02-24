#pragma once

#include "GlideMode.h"
#include "NoiseGenerator.h"
#include "Voice.h"

#include <cstdint>
#include <juce_audio_basics/juce_audio_basics.h>

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
    void controlChange(const uint8_t controller, const uint8_t value);

    void setNoiseMix(const float noiseMix);
    void setOscillatorMix(const float oscillatorMix);
    void setDetune(const float semi, const float cent);
    void setTune(const float tune);
    void setPolyphonic(const bool polyphonic);
    void setOutputLevel(const float outputLevel);
    void setVolumeTrim(const float filterReso);
    void setOutputLevelInstantly(const float outputLevel);
    void setFilterVelocity(const float filterVelocity);
    void setLfoIncrement(const float lfoRateParam, const float inverseSampleRate);
    void setVibratoAmount(const float vibratoParam);
    void setGlide(const int glideMode, const float glideRate, const float glideBend, const float inverseSampleRate);
    void setFilterKeyTracking(const float filterKeyTrackingParam);
    void setFilterQ(const float filterReso);
    void setFilterLfoDepth(const float filterLfoDepth);

    void setEnvelopeDecay(const float decayTime);
    void setEnvelopeAttack(const float attackTime);
    void setEnvelopeSustain(const float sustainLevel);
    void setEnvelopeRelease(const float releaseTime);

  private:
    void startVoice(const size_t voiceIdx, const int note, const int velocity);
    void restartMonoVoice(const int note, const int velocity);
    size_t selectVoiceIndexToUse() const;
    bool isPolyphonic() const;
    bool isInPwmMode() const;
    bool isPlayingLegatoStyle() const;
    void shiftQueuedNotes();
    int nextQueuedNote();
    void processLastNotePriority(const int note);
    void updateLfo();

    void noteOn(const int note, int velocity);
    void noteOff(const int note);

    static constexpr int maxNumVoices_{8};
    int numVoices_{1};
    std::array<Voice, maxNumVoices_> voices_;
    int lastNote_{0};

    float oscillatorMix_{0.0f};
    float tune_{0.0f};
    float detune_{0.0f};
    float pitchBend_{0.0f};
    float modWheel_{0.0f};

    NoiseGenerator noiseGenerator_;
    float noiseMix_{0.0f};

    float volumeTrim_{0.0f};
    juce::LinearSmoothedValue<float> outputLevelSmoother_{0.0f};
    float velocitySensitivity_{0.0f};
    bool ignoreVelocity_{false};

    float envelopeAttack_{0.0f};
    float envelopeDecay_{0.0f};
    float envelopeSustain_{0.0f};
    float envelopeRelease_{0.0f};

    float sampleRate_{0.0f};
    bool sustainPedalPressed_{false};

    static constexpr int lfoMaxSamplesPerUpdate_{32};
    float lfoIncrement_{0.0f};
    float lfo_{0.0f};
    int lfoStep_{0};
    float vibratoAmount_{0.0f};
    float pwmDepth_{0.0f};

    GlideMode glideMode_{GlideMode::Off};
    float glideRate_{0.0f};
    float glideBend_{0.0f};

    float filterKeyTracking_{0.0f};
    float filterQ_{0.0f};
    float resonanceCtl_{0.0f};
    float filterLfoDepth_{0.0f};
    float pressure_{0.0f};
};

} // namespace synth
