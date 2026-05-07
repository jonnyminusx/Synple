#pragma once

#include "ParameterIds.h"
#include "Parameters.h"
#include "synth/Presets.h"
#include "synth/Synth.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

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

    std::atomic<bool> midiLearn;
    std::atomic<float> outputLevelLeft;

  private:
    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override;
    void update();

    void splitBufferByEvents(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
    void handleMidi(const uint8_t data0, const uint8_t data1, const uint8_t data2);
    void render(juce::AudioBuffer<float>& buffer, const int sampleCount, const int bufferOffset);

    synth::Synth synth_;
    Parameters parameters_;
    std::atomic<bool> parametersChanged_{false};
    synth::Presets presets_;
    int currentProgram_;
    std::atomic<uint8_t> midiLearnCC_{0};

    juce::dsp::BallisticsFilter<float> envelopeFollower_;
    juce::AudioBuffer<float> envelopeFollowerOutputBuffer_;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynpleAudioProcessor)
};
