#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include <optional>

class SynpleAudioProcessor;

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

    juce::Slider outputLevelSlider_;
    juce::SliderParameterAttachment outputLevelAttachment_;

    juce::ToggleButton polyphonicToggle_{"Polyphonic"};
    juce::ButtonParameterAttachment polyphonicToggleAttachment_;

    juce::Label glideModeLabel_{"glideModeLabel", "Glide Mode"};
    juce::ComboBox glideModeComboBox_;
    juce::ComboBoxParameterAttachment glideModeAttachment_;

    juce::TextButton runJavaScriptButton_{"Run JavaScript"};
    juce::TextButton emitJavaScriptEventButton_{"Emit JavaScript Event"};
    juce::TextButton midiLearnButton_{"MIDI Learn"};

    juce::Label labelUpdatedFromJavaScript_{"label", "Not updated from JavaScript yet"};

    juce::WebSliderRelay webOutputLevelRelay_;
    juce::WebToggleButtonRelay webPolyphonicToggleRelay_;
    juce::WebComboBoxRelay webGlideModeRelay_;

    juce::WebBrowserComponent webView_;
    juce::WebSliderParameterAttachment webOutputLevelAttachment_;
    juce::WebToggleButtonParameterAttachment webPolyphonicToggleAttachment_;
    juce::WebComboBoxParameterAttachment webGlideModeAttachment_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynpleAudioProcessorEditor)
};
