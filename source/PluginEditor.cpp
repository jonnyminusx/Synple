#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
JLX11AudioProcessorEditor::JLX11AudioProcessorEditor(JLX11AudioProcessor& p) : AudioProcessorEditor(&p), processorRef(p)
{
    juce::LookAndFeel::setDefaultLookAndFeel(&lookAndFeel_);

    outputLevelKnob_.setLabel("Output");
    addAndMakeVisible(outputLevelKnob_);

    filterResoKnob_.setLabel("Filter Reso");
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
