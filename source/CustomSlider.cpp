#include "CustomSlider.h"

CustomSlider::CustomSlider()
{
    setOpaque(true);
}

void CustomSlider::setValue(float newValue)
{
    value = juce::jlimit(0.0f, 1.0f, newValue);
    repaint();
}

float CustomSlider::getValue() const { return value; }

void CustomSlider::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Background
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(bounds);

    // Slider track
    g.setColour(juce::Colours::grey);
    auto trackBounds = bounds.reduced(2);
    g.fillRect(trackBounds);

    // Slider thumb
    g.setColour(juce::Colours::white);
    float thumbWidth = 10.0f;
    float thumbX = value * (getWidth() - thumbWidth);
    g.fillRect(thumbX, 0.0f, thumbWidth, static_cast<float>(getHeight()));
}

void CustomSlider::mouseDown(const juce::MouseEvent& e)
{
    mouseDrag(e);
}

void CustomSlider::mouseDrag(const juce::MouseEvent& e)
{
    auto newValue = (e.position.x - 5.0f) / (getWidth() - 10.0f);
    setValue(newValue);
    if (onValueChanged) onValueChanged(value);
}

