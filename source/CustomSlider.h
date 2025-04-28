#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class CustomSlider : public juce::Component
{
public:
    CustomSlider();

    void setValue(float newValue);
    float getValue() const;

    std::function<void(float)> onValueChanged;

private:
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;

    float value = 0.5f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomSlider)
};
