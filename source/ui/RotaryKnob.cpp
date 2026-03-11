#include "RotaryKnob.h"

namespace ui
{

namespace
{

constexpr int labelHeight = 15;
constexpr int textBoxHeight = 20;
constexpr bool debug = false;

} // namespace

RotaryKnob::RotaryKnob()
{
    slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, textBoxHeight);
    addAndMakeVisible(slider);
    setBounds(0, 0, 100, 120);
}

void RotaryKnob::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setFont(15.0f);
    g.setColour(juce::Colours::white);
    auto bounds = getLocalBounds();
    g.drawText(label_, juce::Rectangle<int>{0, 0, bounds.getWidth(), labelHeight}, juce::Justification::centred);

    if (debug)
    {
        g.setColour(juce::Colours::red);
        g.drawRect(getLocalBounds(), 1);
    }
}

void RotaryKnob::resized()
{
    auto bounds = getLocalBounds();
    slider.setBounds(0, labelHeight, bounds.getWidth(), bounds.getHeight() - labelHeight);
}

void RotaryKnob::setLabel(const juce::String& newLabel)
{
    label_ = newLabel;
}

} // namespace ui
