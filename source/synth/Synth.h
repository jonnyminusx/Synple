#pragma once

#include "NoiseGenerator.h"
#include "Parameters.h"
#include "Voice.h"
#include "midi/MidiProcessor.h"
#include "midi/NoteHandler.h"

#include <array>
#include <cstddef>
#include <juce_audio_basics/juce_audio_basics.h>

namespace synth
{

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
    void sustainPedalReleased() override;

    midi::MidiProcessor& midiProcessor() { return midiProcessor_; }

    void setParameters(const Parameters& parameters);
    void setOutputLevelInstantly(const float outputLevel);

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

    juce::LinearSmoothedValue<float> outputLevelSmoother_{0.0f};

    float sampleRate_{44100.0f};

    static constexpr int lfoMaxSamplesPerUpdate_{32};
    float lfo_{0.0f};
    int lfoStep_{0};

    float filterZip_{0.0f};
};

} // namespace synth
