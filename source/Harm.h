#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class Harm : public juce::Component
{
public:
    static constexpr int numValues = 8;
    //==============================================================================

    Harm(juce::Colour barColor = juce::Colours::blue) : barColour(barColor)
    {
        initializeData();
        setOpaque(true);
    }
    ~Harm() override {}

    //==============================================================================

    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================

    void updateContent();
    void addRow(float value);
    void clear();

    // Add getter/setter methods
    float getValue(int index) const 
    { 
        return harmData[index]; 
    }
    
    // Add callback type definition
    std::function<void()> onValueChange;

    void setValue(int index, float value)
    {
        if (index >= 0 && index < numValues)
        {
            harmData.set(index, juce::jlimit(0.0f, 1.0f, value));
            repaint();
            // Call the callback when value changes
            if (onValueChange != nullptr)
                onValueChange();
        }
    }

    // Add serialization method
    juce::Array<float> getHarmonicData() const
    {
        return harmData;
    }

    void setHarmonicData(const juce::Array<float>& data)
    {
        harmData.clear();
        harmData = data;
        repaint();
    }

private:
    float values[numValues] = { 0.0f };  // Initialize array with zeros

    //==============================================================================

    struct TableConfig
    {
        int rowHeight = 25;
        int barSpacing = 2;
        juce::Colour rowColour { juce::Colours::lightgrey };
        juce::Colour alternateRowColour { juce::Colours::white };
    } config;

    // Your data model
    juce::Array<float> harmData;

    void initializeData()
    {
        // Initialize both arrays
        for (int i = 0; i < numValues; ++i)
        {
            harmData.add(i * 0.1f);
            values[i] = i * 0.1f;
        }
    }

    void drawRows(juce::Graphics& g);
    bool isDragging = false;
    float valueFromY(float y) const;
    int getBarAtPosition(float x);    // Add this line
    
    // Override mouse events
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    juce::Colour barColour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Harm)
};