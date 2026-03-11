#include <juce_gui_basics/juce_gui_basics.h>

namespace ui
{

class RotaryKnob : public juce::Component
{
  public:
    RotaryKnob();
    ~RotaryKnob() final = default;

    void paint(juce::Graphics&) final;
    void resized() final;

    void setLabel(const juce::String& newLabel);
    juce::Slider slider;

  private:
    juce::String label_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RotaryKnob)
};

} // namespace ui
