#include "Harm.h"

void Harm::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    drawRows(g);
}

void Harm::drawRows(juce::Graphics& g)
{
    const float availableWidth = static_cast<float>(getWidth());
    const float barWidth = (availableWidth / harmData.size()) - config.barSpacing;
    const int contentHeight = getHeight();
    
    g.setColour(barColour);
    
    for (int i = 0; i < harmData.size(); ++i)
    {
        const float value = harmData[i];
        const float xPos = i * (barWidth + config.barSpacing);
        const int barHeight = juce::roundToInt(value * contentHeight);
        const int yPos = contentHeight - barHeight;
        
        g.fillRect(xPos, static_cast<float>(yPos), barWidth, static_cast<float>(barHeight));
    }
}

void Harm::updateContent()
{
    repaint();
}

void Harm::addRow(float value)
{
    harmData.add(value);
    repaint();
}

void Harm::clear()
{
    harmData.clear();
    repaint();
}

void Harm::resized()
{
    repaint();
}

int Harm::getBarAtPosition(float x)
{
    const float barWidth = (static_cast<float>(getWidth()) / harmData.size()) - config.barSpacing;
    
    for (int i = 0; i < harmData.size(); ++i)
    {
        float barX = i * (barWidth + config.barSpacing);
        if (x >= barX && x < barX + barWidth)
        {
            return i;
        }
    }
    return -1;
}

float Harm::valueFromY(float y) const
{
    float value = 1.0f - (y / getHeight());
    return juce::jlimit(0.0f, 1.0f, value);
}

void Harm::mouseDown(const juce::MouseEvent& e)
{
    if (getBarAtPosition(e.position.x) != -1)
    {
        isDragging = true;
    }
}

void Harm::mouseDrag(const juce::MouseEvent& e)
{
    if (isDragging)
    {
        int barIndex = getBarAtPosition(e.position.x);
        if (barIndex != -1)
        {
            float newValue = valueFromY(e.position.y);
            harmData.set(barIndex, newValue);
            DBG("Bar " << barIndex << " value: " << newValue);
            repaint();
        }
    }
}

void Harm::mouseUp(const juce::MouseEvent& e)
{
    isDragging = false;
}
