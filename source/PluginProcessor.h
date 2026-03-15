#pragma once

#include "synth/Synth.h"
#include <juce_audio_processors/juce_audio_processors.h>

class Preset;

namespace parameter_id
{

#define PARAMETER_ID(str) const juce::ParameterID str(#str, 1);

PARAMETER_ID(oscMix)
PARAMETER_ID(oscTune)
PARAMETER_ID(oscFine)
PARAMETER_ID(glideMode)
PARAMETER_ID(glideRate)
PARAMETER_ID(glideBend)
PARAMETER_ID(filterFreq)
PARAMETER_ID(filterReso)
PARAMETER_ID(filterEnv)
PARAMETER_ID(filterLFO)
PARAMETER_ID(filterVelocity)
PARAMETER_ID(filterAttack)
PARAMETER_ID(filterDecay)
PARAMETER_ID(filterSustain)
PARAMETER_ID(filterRelease)
PARAMETER_ID(envAttack)
PARAMETER_ID(envDecay)
PARAMETER_ID(envSustain)
PARAMETER_ID(envRelease)
PARAMETER_ID(lfoRate)
PARAMETER_ID(vibrato)
PARAMETER_ID(noise)
PARAMETER_ID(octave)
PARAMETER_ID(tuning)
PARAMETER_ID(outputLevel)
PARAMETER_ID(polyMode)

#undef PARAMETER_ID

} // namespace parameter_id

//==============================================================================
class JLX11AudioProcessor final : public juce::AudioProcessor, private juce::ValueTree::Listener
{
  public:
    //==============================================================================
    JLX11AudioProcessor();
    ~JLX11AudioProcessor() override;

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

    juce::AudioProcessorValueTreeState& getApvts();

    std::atomic<bool> midiLearn;

  private:
    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override;
    void update();
    void createPrograms();

    void splitBufferByEvents(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
    void handleMidi(const uint8_t data0, const uint8_t data1, const uint8_t data2);
    void render(juce::AudioBuffer<float>& buffer, const int sampleCount, const int bufferOffset);
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    synth::Synth synth_;

    juce::AudioParameterFloat* oscMixParam_;
    juce::AudioParameterFloat* oscTuneParam_;
    juce::AudioParameterFloat* oscFineParam_;
    juce::AudioParameterChoice* glideModeParam_;
    juce::AudioParameterFloat* glideRateParam_;
    juce::AudioParameterFloat* glideBendParam_;
    juce::AudioParameterFloat* filterFreqParam_;
    juce::AudioParameterFloat* filterResoParam_;
    juce::AudioParameterFloat* filterEnvParam_;
    juce::AudioParameterFloat* filterLFOParam_;
    juce::AudioParameterFloat* filterVelocityParam_;
    juce::AudioParameterFloat* filterAttackParam_;
    juce::AudioParameterFloat* filterDecayParam_;
    juce::AudioParameterFloat* filterSustainParam_;
    juce::AudioParameterFloat* filterReleaseParam_;
    juce::AudioParameterFloat* envAttackParam_;
    juce::AudioParameterFloat* envDecayParam_;
    juce::AudioParameterFloat* envSustainParam_;
    juce::AudioParameterFloat* envReleaseParam_;
    juce::AudioParameterFloat* lfoRateParam_;
    juce::AudioParameterFloat* vibratoParam_;
    juce::AudioParameterFloat* noiseParam_;
    juce::AudioParameterFloat* octaveParam_;
    juce::AudioParameterFloat* tuningParam_;
    juce::AudioParameterFloat* outputLevelParam_;
    juce::AudioParameterChoice* polyModeParam_;

    juce::AudioProcessorValueTreeState apvts_{*this, nullptr, "Parameters", createParameterLayout()};
    std::atomic<bool> parametersChanged_{false};
    std::vector<Preset> presets_;
    int currentProgram_;
    std::atomic<uint8_t> midiLearnCC_{0};

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JLX11AudioProcessor)
};
