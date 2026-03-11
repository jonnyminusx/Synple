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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JLX11AudioProcessorEditor)
};
