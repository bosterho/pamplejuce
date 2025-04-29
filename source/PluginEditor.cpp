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

    // Setup preset browser
    currentPresetDirectory = juce::File("C:/ProgramData/Additive Midi/Factory presets");
    updatePresetList();
    
    addAndMakeVisible(nextPresetButton);
    addAndMakeVisible(prevPresetButton);
    
    nextPresetButton.onClick = [this]() { loadNextPreset(); };
    prevPresetButton.onClick = [this]() { loadPrevPreset(); };
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
    
    auto buttonWidth = 60;
    auto buttonSpacing = 10;
    auto buttonsY = buttonArea.getY() + 10;
    
    // Position the preset browser buttons
    prevPresetButton.setBounds(10, buttonsY, buttonWidth, 30);
    nextPresetButton.setBounds(buttonWidth + buttonSpacing + 10, buttonsY, buttonWidth, 30);
    
    // Adjust existing save/load buttons position
    auto centerButtonsX = (getWidth() - (2 * 100 + buttonSpacing)) / 2;
    savePresetButton.setBounds(centerButtonsX, buttonsY, 100, 30);
    loadPresetButton.setBounds(centerButtonsX + 100 + buttonSpacing, buttonsY, 100, 30);
    
    // Divide remaining space horizontally for harm1, combo, and harm2
    auto thirdWidth = area.getWidth() / 3;
    harm1.setBounds(area.removeFromLeft(thirdWidth).reduced(10));
    combo.setBounds(area.removeFromLeft(thirdWidth).reduced(10));
    harm2.setBounds(area.reduced(10));
}

void PluginEditor::savePreset()
{
    dialogWindow = std::make_unique<juce::AlertWindow>(
        "Save Preset",
        "help text",
        juce::MessageBoxIconType::NoIcon);  // Changed from QuestionIcon to NoIcon

    // First add the text editor and buttons
    dialogWindow->addTextEditor("presetName", "New Preset");
    dialogWindow->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey, 0, 0));
    dialogWindow->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey, 0, 0));

    // Now style everything after components are created
    dialogWindow->setColour(juce::AlertWindow::backgroundColourId, juce::Colour(0xFF191919));
    dialogWindow->setColour(juce::AlertWindow::textColourId, juce::Colours::white);
    dialogWindow->setColour(juce::AlertWindow::outlineColourId, juce::Colours::grey);
    
    // Style text editor after it exists
    if (auto* editor = dialogWindow->getTextEditor("presetName"))
    {
        editor->setColour(juce::TextEditor::backgroundColourId, juce::Colours::black);
        editor->setColour(juce::TextEditor::textColourId, juce::Colours::white);
        editor->setColour(juce::TextEditor::outlineColourId, juce::Colours::grey);
        editor->setFont(juce::Font(16.0f));
    }
    

    // Add this code to style the buttons properly
    for (auto* child : dialogWindow->getChildren())
    {
        if (auto* button = dynamic_cast<juce::TextButton*>(child))
        {
            button->setColour(juce::TextButton::buttonColourId, juce::Colours::black);
            button->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
            button->setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFF303030));
            button->setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFF303030));
        }
    }

    // Center dialog relative to plugin window
    auto pluginBounds = getBounds();
    auto dialogBounds = dialogWindow->getBounds();
    auto screenPos = getScreenPosition();
    
    // Center horizontally and position slightly above vertical center
    int x = screenPos.getX() + (pluginBounds.getWidth() - dialogBounds.getWidth()) / 2;
    int y = screenPos.getY() + (pluginBounds.getHeight() - dialogBounds.getHeight()) / 3;
    
    dialogWindow->setBounds(x, y, dialogBounds.getWidth(), dialogBounds.getHeight());

    // Show modal dialog
    dialogWindow->enterModalState(true, juce::ModalCallbackFunction::create(
        [this](int result)  // Changed this line
        {
            if (result == 1) // "OK" was clicked
            {
                juce::String presetName = dialogWindow->getTextEditorContents("presetName");
                if (presetName.isEmpty())
                    return;

                // Ensure name ends with .preset
                if (!presetName.endsWithIgnoreCase(".preset"))
                    presetName += ".preset";

                // Create file in factory presets directory
                auto presetFile = currentPresetDirectory.getChildFile(presetName);

                PresetData data;
                data.harm1Data = harm1.getHarmonicData();
                data.harm2Data = harm2.getHarmonicData();
                data.comboData = combo.getHarmonicData();
                data.morphValue = static_cast<float>(morphSlider.getValue());
                
                data.saveToFile(presetFile);

                // Refresh the preset list
                updatePresetList();
            }
            dialogWindow.reset();  // Add this line to clean up
        }
    ));
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

void PluginEditor::updatePresetList()
{
    presetFiles.clear();
    currentPresetDirectory.findChildFiles(presetFiles, juce::File::findFiles, false, "*.preset");
    presetFiles.sort();  // Sort alphabetically
}

void PluginEditor::loadNextPreset()
{
    if (presetFiles.isEmpty()) return;
    
    currentPresetIndex = (currentPresetIndex + 1) % presetFiles.size();
    loadPresetAtIndex(currentPresetIndex);
}

void PluginEditor::loadPrevPreset()
{
    if (presetFiles.isEmpty()) return;
    
    currentPresetIndex--;
    if (currentPresetIndex < 0) currentPresetIndex = presetFiles.size() - 1;
    loadPresetAtIndex(currentPresetIndex);
}

void PluginEditor::loadPresetAtIndex(int index)
{
    if (index >= 0 && index < presetFiles.size())
    {
        auto data = PresetData::loadFromFile(presetFiles[index]);
        
        // Apply the loaded data
        harm1.setHarmonicData(data.harm1Data);
        harm2.setHarmonicData(data.harm2Data);
        combo.setHarmonicData(data.comboData);
        morphSlider.setValue(data.morphValue, juce::sendNotification);
    }
}