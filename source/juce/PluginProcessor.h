#pragma once

#include "ParameterIds.h"
#include "Parameters.h"
#include "midi/MidiLearnMap.h"
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

    void beginMidiLearn(const juce::String& paramId);
    void cancelMidiLearn();
    void clearMidiLearn(const juce::String& paramId);
    uint8_t getMidiLearnCC(const juce::String& paramId) const;
    juce::String getMidiLearnParamId() const;
    juce::var getMidiLearnState() const;

  private:
    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override;
    void update();

    void splitBufferByEvents(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
    void handleMidi(uint8_t data0, uint8_t data1, uint8_t data2);
    void render(juce::AudioBuffer<float>& buffer, int sampleCount, int bufferOffset);

    void initialiseLearnableParams();

    synth::Synth synth_;
    Parameters parameters_;
    std::atomic<bool> parametersChanged_{false};
    synth::Presets presets_;
    int currentProgram_;

    midi::MidiLearnMap midiLearnMap_{};
    std::array<juce::RangedAudioParameter*, midi::MidiLearnMap::kNumParams> learnableParams_{};

    std::vector<std::span<float>> renderChannels_;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynpleAudioProcessor)
};
