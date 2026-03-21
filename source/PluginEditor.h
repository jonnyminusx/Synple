#pragma once

#include "PluginProcessor.h"

//==============================================================================
class JLX11AudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Button::Listener, juce::Timer
{
  public:
    explicit JLX11AudioProcessorEditor(JLX11AudioProcessor&);
    ~JLX11AudioProcessorEditor() override;

    void resized() override;
    void buttonClicked(juce::Button* button) override;
    void timerCallback() override;

  private:
    using Resource = juce::WebBrowserComponent::Resource;
    std::optional<Resource> getResource(const juce::String& url);

    JLX11AudioProcessor& processorRef;
    juce::WebBrowserComponent webView_;

    juce::TextButton runJavaScriptButton_{"Run JavaScript"};
    juce::TextButton emitJavaScriptEventButton_{"Emit JavaScript Event"};
    juce::TextButton midiLearnButton_{"MIDI Learn"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JLX11AudioProcessorEditor)
};
