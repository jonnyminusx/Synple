#pragma once

#include "ParameterIds.h"
#include "Parameters.h"
#include "synth/Presets.h"
#include "synth/Synth.h"
#include <array>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <span>
#include <vector>

//==============================================================================
class SynpleAudioProcessor final : public juce::AudioProcessor, private juce::ValueTree::Listener
{
  public:
    //==============================================================================
    SynpleAudioProcessor();
    ~SynpleAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    void reset() override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    using AudioProcessor::getParameter;
    juce::RangedAudioParameter& getParameter(const juce::ParameterID& id);

    std::atomic<float> outputLevelLeft;

    void beginMidiLearn(const juce::String& paramId);
    void cancelMidiLearn();
    void clearMidiLearn(const juce::String& paramId);
    uint8_t getMidiLearnCC(const juce::String& paramId) const;
    juce::String getMidiLearnParamId() const;

    static constexpr uint8_t kCCUnassigned{0xFF};
    static constexpr size_t kNumLearnableParams{34};
    static constexpr std::array<const char*, kNumLearnableParams> kLearnableParamIds{{
        "glideMode",    "glideRate",      "glideBend",      "filterFreq",     "filterReso",    "filterEnv",
        "filterLFO",    "filterVelocity", "filterAttack",   "filterDecay",    "filterSustain", "filterRelease",
        "envAttack",    "envDecay",       "envSustain",     "envRelease",     "lfoRate",       "vibrato",
        "noise",        "octave",         "tuning",         "outputLevel",    "polyMode",      "pwmDepth",
        "osc1Volume",   "osc2Volume",     "osc1Tune",       "osc2Tune",       "osc1Fine",      "osc2Fine",
        "osc1Waveform", "osc2Waveform",   "osc1PulseWidth", "osc2PulseWidth",
    }};

  private:
    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override;
    void update();

    void splitBufferByEvents(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
    void handleMidi(uint8_t data0, uint8_t data1, uint8_t data2);
    void render(juce::AudioBuffer<float>& buffer, int sampleCount, int bufferOffset);

    void initialiseMidiLearnMap();
    int midiLearnIndexForId(const juce::String& paramId) const;

    synth::Synth synth_;
    Parameters parameters_;
    std::atomic<bool> parametersChanged_{false};
    synth::Presets presets_;
    int currentProgram_;

    std::array<juce::RangedAudioParameter*, kNumLearnableParams> learnableParams_{};
    std::array<std::atomic<uint8_t>, kNumLearnableParams> midiCCMap_;
    std::atomic<int> midiLearnIndex_{-1};

    juce::dsp::BallisticsFilter<float> envelopeFollower_;
    juce::AudioBuffer<float> envelopeFollowerOutputBuffer_;
    std::vector<std::span<float>> renderChannels_;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynpleAudioProcessor)
};
