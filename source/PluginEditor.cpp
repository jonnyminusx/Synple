#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
JLX11AudioProcessorEditor::JLX11AudioProcessorEditor(JLX11AudioProcessor& p) : AudioProcessorEditor(&p), processorRef(p)
{
    outputLevelKnob_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outputLevelKnob_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    addAndMakeVisible(outputLevelKnob_);

    filterResoKnob_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    filterResoKnob_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    addAndMakeVisible(filterResoKnob_);

    setSize(600, 400);
}

JLX11AudioProcessorEditor::~JLX11AudioProcessorEditor()
{
}

//==============================================================================
void JLX11AudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void JLX11AudioProcessorEditor::resized()
{
    outputLevelKnob_.setBounds(20, 20, 100, 120);
    filterResoKnob_.setBounds(120, 20, 100, 120);
}
