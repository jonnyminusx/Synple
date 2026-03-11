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

    polyModeButton_.setButtonText("Poly");
    polyModeButton_.setClickingTogglesState(true);
    addAndMakeVisible(polyModeButton_);

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
    juce::Rectangle r(20, 20, 100, 120);
    outputLevelKnob_.setBounds(r);

    r = r.withX(r.getRight() + 20);
    filterResoKnob_.setBounds(r);

    polyModeButton_.setSize(80, 30);
    polyModeButton_.setCentrePosition(r.withX(r.getRight()).getCentre());
}
