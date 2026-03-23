#pragma once

#include "PluginProcessor.h"

//==============================================================================
class SynpleAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Button::Listener, juce::Timer
{
  public:
    explicit SynpleAudioProcessorEditor(SynpleAudioProcessor&);
    ~SynpleAudioProcessorEditor() override;

    void resized() override;
    void buttonClicked(juce::Button* button) override;
    void timerCallback() override;

  private:
    using Resource = juce::WebBrowserComponent::Resource;
    std::optional<Resource> getResource(const juce::String& url);

    void nativeFunction(const juce::Array<juce::var>& args,
                        juce::WebBrowserComponent::NativeFunctionCompletion completion);

    SynpleAudioProcessor& processorRef;
    juce::WebBrowserComponent webView_;

    juce::TextButton runJavaScriptButton_{"Run JavaScript"};
    juce::TextButton emitJavaScriptEventButton_{"Emit JavaScript Event"};
    juce::TextButton midiLearnButton_{"MIDI Learn"};

    juce::Label labelUpdatedFromJavaScript_{"label", "Not updated from JavaScript yet"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynpleAudioProcessorEditor)
};
