#include "Harm.h"

//==============================================================================
Harm::Harm()
{
    // Initialize your table
    addAndMakeVisible(tableListBox);
    
    // Set this instance as the model for the table
    tableListBox.setModel(this);
    
    // Configure the columns
    setupColumns();
}

Harm::~Harm()
{
    // Cleanup if needed
}

void Harm::paint(juce::Graphics& g)
{
    // Paint any custom background if needed
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void Harm::resized()
{
    // Size the table to fill the entire component
    tableListBox.setBounds(getLocalBounds());
}

int Harm::getNumRows()
{
    // Return the number of rows in your data model
    return 0; // Replace with actual row count
}

void Harm::paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    // Paint the background of each row
    if (rowIsSelected)
        g.fillAll(juce::Colours::lightblue);
    else if (rowNumber % 2)
        g.fillAll(juce::Colours::white);
    else
        g.fillAll(juce::Colour(0xffeeeeee));
}

void Harm::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    // Paint the contents of a cell
    g.setColour(rowIsSelected ? juce::Colours::darkblue : juce::Colours::black);
    g.setFont(14.0f);
    
    // Here you would draw the actual cell content based on your data model
    g.drawText("Cell (" + juce::String(rowNumber) + "," + juce::String(columnId) + ")", 
               2, 0, width - 4, height, juce::Justification::centredLeft, true);
}

juce::Component* Harm::refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, 
                                                juce::Component* existingComponentToUpdate)
{
    // Return a component to display in the cell, or nullptr for a regular cell
    return nullptr;
}

void Harm::sortOrderChanged(int newSortColumnId, bool isForwards)
{
    // Handle sorting when a column header is clicked
    // Sort your data model here
    
    // Then update the display
    tableListBox.updateContent();
}

void Harm::updateContent()
{
    // Call this when your data changes
    tableListBox.updateContent();
    repaint();
}

void Harm::addRow(/* your parameters here */)
{
    // Add a row to your data model
    // For example: tableData.add(newItem);
    
    // Then update the display
    updateContent();
}

void Harm::clear()
{
    // Clear your data model
    // For example: tableData.clear();
    
    // Then update the display
    updateContent();
}

void Harm::setupColumns()
{
    // Configure your table columns
    auto& header = tableListBox.getHeader();
    
    // Clear any existing columns
    header.removeAllColumns();
    
    // Add columns with IDs, names, and widths
    // Column IDs should start from 1 (0 is reserved)
    header.addColumn("Column 1", 1, 100);
    header.addColumn("Column 2", 2, 100);
    header.addColumn("Column 3", 3, 100);
    
    // Additional column settings
    header.setStretchToFitActive(true);
}