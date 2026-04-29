#pragma once

#include "NoiseGenerator.h"
#include "Parameters.h"
#include "Voice.h"
#include "midi/MidiProcessor.h"

#include <cstdint>
#include <juce_audio_basics/juce_audio_basics.h>

namespace synth
{

struct ADSR;
class AudioBuffer;

class Synth : public midi::NoteHandler
{
  public:
    Synth();

    void allocateResources(const float sampleRate, const int samplesPerBlock);
    void deallocateResources() const;
    void reset();
    void render(AudioBuffer& audioBuffer);

    // midi::NoteHandler interface
    void noteOn(int note, int velocity) override;
    void noteOff(int note) override;
    void allNotesOff() override;

    midi::MidiProcessor& midiProcessor() { return midiProcessor_; }

    void setNoiseMix(const float noiseMix);
    void setOscillatorMix(const float oscillatorMix);
    void setDetune(const float detune);
    void setTune(const float tune);
    void setPolyphonic(const bool polyphonic);
    void setOutputLevel(const float outputLevel);
    void setVolumeTrim(const float volumeTrim);
    void setOutputLevelInstantly(const float outputLevel);
    void setIgnoreVelocity(const bool ignoreVelocity);
    void setVelocitySensitivity(const float velocitySensitivity);
    void setLfoIncrement(const float lfoIncrement);
    void setVibratoAmount(const float vibratoAmount);
    void setPwmDepth(const float pwmDepth);
    void setGlideMode(const int glideMode);
    void setGlideRate(const float glideRate);
    void setGlideBend(const float glideBend);
    void setFilterKeyTracking(const float filterKeyTracking);
    void setFilterQ(const float filterQ);
    void setFilterLfoDepth(const float filterLfoDepth);
    void setFilterEnvelope(const ADSR& adsr);
    void setFilterEnvelopeDepth(const float envDepth);
    void setEnvelope(const ADSR& adsr);

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

    static constexpr int maxNumVoices_{8};
    int numVoices_{1};
    std::array<Voice, maxNumVoices_> voices_;
    int lastNote_{0};

    Parameters parameters_{};

    midi::MidiProcessor midiProcessor_;

    NoiseGenerator noiseGenerator_;
    float noiseMix_{0.0f};

    juce::LinearSmoothedValue<float> outputLevelSmoother_{0.0f};
    bool ignoreVelocity_{false};

    float sampleRate_{44100.0f};

    static constexpr int lfoMaxSamplesPerUpdate_{32};
    float lfoIncrement_{0.0f};
    float lfo_{0.0f};
    int lfoStep_{0};
    float vibratoAmount_{0.0f};
    float pwmDepth_{0.0f};

    float glideRate_{0.0f};

    float filterKeyTracking_{0.0f};
    float filterQ_{0.0f};
    float filterLfoDepth_{0.0f};
    float filterZip_{0.0f};
    float filterEnvDepth_{0.0f};
};

} // namespace synth
