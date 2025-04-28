#include "PluginEditor.h"

PluginEditor::PluginEditor(PluginProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p),
      resizer(this, &constrainer)
{
    // Initialize the background drawable
    background = juce::Drawable::createFromImageData(BinaryData::background_svg, 
                                                   BinaryData::background_svgSize);

    // addAndMakeVisible (inspectButton);

    addAndMakeVisible(harm1);
    addAndMakeVisible(harm2);
    addAndMakeVisible(combo);
    
    // Configure the slider
    morphSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    morphSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(morphSlider);
    
    // Add APVTS attachment
    morphAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.getAPVTS(), "Morph", morphSlider);
    
    // Update values when slider changes
    morphSlider.onValueChange = [this]() {
        float value = static_cast<float>(morphSlider.getValue());
        for (int i = 0; i < Harm::numValues; ++i)
        {
            float v1 = harm1.getValue(i);
            float v2 = harm2.getValue(i);
            float interpolatedValue = v1 * (1.0f - value) + v2 * value;
            combo.setValue(i, interpolatedValue);
        }
    };
    
    // Set up resizing constraints with fixed aspect ratio
    constrainer.setFixedAspectRatio(800.0f / 450.0f);  // Based on initial size
    constrainer.setMinimumSize(400, 225);  // Maintains aspect ratio
    constrainer.setMaximumSize(1200, 675); // Maintains aspect ratio
    
    setResizable(true, true);
    setConstrainer(&constrainer);
    addAndMakeVisible(resizer);

    setSize(800, 450);

    addAndMakeVisible(savePresetButton);
    savePresetButton.onClick = [this]() { savePreset(); };
    
    addAndMakeVisible(loadPresetButton);
    loadPresetButton.onClick = [this]() { loadPreset(); };
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    g.fillAll(juce::Colours::black);
    
    // Only draw background if it exists
    if (background != nullptr)
    {
        background->drawWithin(g, bounds.toFloat(),
            juce::RectanglePlacement::centred, 1.0f);
    }
}

void PluginEditor::resized()
{
    auto area = getLocalBounds();
    
    // Place resizer in bottom-right corner
    resizer.setBounds(area.removeFromRight(10).removeFromBottom(10));
    
    // Reserve space for slider at bottom
    auto sliderArea = area.removeFromBottom(50);
    morphSlider.setBounds(sliderArea.reduced(10));

    // Space for inspect button
    auto buttonArea = area.removeFromBottom(50);
    inspectButton.setBounds(buttonArea.withSizeKeepingCentre(100, 50));
    
    auto buttonWidth = 100;
    auto buttonSpacing = 10;
    auto buttonsX = (getWidth() - (2 * buttonWidth + buttonSpacing)) / 2;
    
    savePresetButton.setBounds(buttonsX, buttonArea.getY() + 10, buttonWidth, 30);
    loadPresetButton.setBounds(buttonsX + buttonWidth + buttonSpacing, 
                             buttonArea.getY() + 10, buttonWidth, 30);
    
    // Divide remaining space horizontally for harm1, combo, and harm2
    auto thirdWidth = area.getWidth() / 3;
    harm1.setBounds(area.removeFromLeft(thirdWidth).reduced(10));
    combo.setBounds(area.removeFromLeft(thirdWidth).reduced(10));
    harm2.setBounds(area.reduced(10));
}

void PluginEditor::savePreset()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Save Preset",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
        "*.preset");

    auto chooserFlags = juce::FileBrowserComponent::saveMode 
                     | juce::FileBrowserComponent::canSelectFiles 
                     | juce::FileBrowserComponent::warnAboutOverwriting;

    fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
    {
        if (fc.getURLResult().isLocalFile())
        {
            auto file = fc.getResult();
            
            PresetData data;
            data.harm1Data = harm1.getHarmonicData();
            data.harm2Data = harm2.getHarmonicData();
            data.comboData = combo.getHarmonicData();
            data.morphValue = static_cast<float>(morphSlider.getValue());
            
            data.saveToFile(file);
        }
        fileChooser.reset();
    });
}

void PluginEditor::loadPreset()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Load Preset",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
        "*.preset");

    auto chooserFlags = juce::FileBrowserComponent::openMode 
                     | juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
    {
        if (fc.getURLResult().isLocalFile())
        {
            auto file = fc.getResult();
            auto data = PresetData::loadFromFile(file);
            
            // Apply the loaded data
            harm1.setHarmonicData(data.harm1Data);
            harm2.setHarmonicData(data.harm2Data);
            combo.setHarmonicData(data.comboData);
            morphSlider.setValue(data.morphValue, juce::sendNotification);
        }
        fileChooser.reset();
    });
}

juce::Array<float> PluginEditor::getComboHarmonicData() const
{
    return combo.getHarmonicData();
}