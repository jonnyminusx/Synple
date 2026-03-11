#pragma once

#include "PluginProcessor.h"

//==============================================================================
class JLX11AudioProcessorEditor final : public juce::AudioProcessorEditor
{
  public:
    explicit JLX11AudioProcessorEditor(JLX11AudioProcessor&);
    ~JLX11AudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

  private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    JLX11AudioProcessor& processorRef;

    juce::Slider outputLevelKnob_;
    juce::AudioProcessorValueTreeState::SliderAttachment outputLevelAttachment_{
        processorRef.getApvts(), parameter_id::outputLevel.getParamID(), outputLevelKnob_};

    juce::Slider filterResoKnob_;
    juce::AudioProcessorValueTreeState::SliderAttachment filterResoAttachment{
        processorRef.getApvts(), parameter_id::filterReso.getParamID(), filterResoKnob_};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JLX11AudioProcessorEditor)
};
