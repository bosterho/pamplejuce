#pragma once
#include <JuceHeader.h>

//==============================================================================
/**
    A custom table component for displaying tabular data.
*/
class Harm  : public juce::Component,
                 public juce::TableListBoxModel
{
public:
    //==============================================================================
    Harm();
    ~Harm() override;

    //==============================================================================
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    // TableListBoxModel overrides
    int getNumRows() override;
    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    
    // Optional TableListBoxModel methods you might want to override
    juce::Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, 
                                            juce::Component* existingComponentToUpdate) override;
    void sortOrderChanged(int newSortColumnId, bool isForwards) override;
    
    //==============================================================================
    void updateContent();
    void addRow(/* your parameters here */);
    void clear();

private:
    //==============================================================================
    juce::TableListBox tableListBox;
    
    // Your data model
    // For example:
    // juce::Array<MyDataType> tableData;
    
    // Column configuration
    void setupColumns();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Harm)
};