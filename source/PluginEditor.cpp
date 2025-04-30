#include "PluginEditor.h"

PluginEditor::PluginEditor(PluginProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p),
      resizer(this, &constrainer)
{
    // Initialize the background drawable
    background = juce::Drawable::createFromImageData(BinaryData::background_svg, 
                                                   BinaryData::background_svgSize);

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
    
    // Initialize harmonics with stored values
    harm1.setHarmonicData(processorRef.getHarm1Data());
    harm2.setHarmonicData(processorRef.getHarm2Data());
    combo.setHarmonicData(processorRef.getComboData());
    
    // Add value change listeners for direct manipulation of harmonics
    harm1.onValueChange = [this]() {
        processorRef.setHarmonicData(
            harm1.getHarmonicData(),
            harm2.getHarmonicData(),
            combo.getHarmonicData()
        );
    };
    
    harm2.onValueChange = [this]() {
        processorRef.setHarmonicData(
            harm1.getHarmonicData(),
            harm2.getHarmonicData(),
            combo.getHarmonicData()
        );
    };
    
    // Existing morph slider callback
    morphSlider.onValueChange = [this]() {
        float value = static_cast<float>(morphSlider.getValue());
        for (int i = 0; i < Harm::numValues; ++i)
        {
            float v1 = harm1.getValue(i);
            float v2 = harm2.getValue(i);
            float interpolatedValue = v1 * (1.0f - value) + v2 * value;
            combo.setValue(i, interpolatedValue);
        }
        // Store updated values in processor
        processorRef.setHarmonicData(
            harm1.getHarmonicData(),
            harm2.getHarmonicData(),
            combo.getHarmonicData()
        );
    };
    
    // Set up resizing constraints
    constrainer.setFixedAspectRatio(800.0f / 450.0f);
    constrainer.setMinimumSize(400, 225);
    constrainer.setMaximumSize(1200, 675);
    
    setResizable(true, true);
    setConstrainer(&constrainer);
    addAndMakeVisible(resizer);

    setSize(800, 450);

    addAndMakeVisible(savePresetButton);
    savePresetButton.onClick = [this]() { savePreset(); };
    
    addAndMakeVisible(loadPresetButton);
    loadPresetButton.onClick = [this]() { loadPreset(); };

    currentPresetDirectory = juce::File::getSpecialLocation(juce::File::commonApplicationDataDirectory)
                            .getChildFile(JucePlugin_Manufacturer)
                            .getChildFile(JucePlugin_Name)
                            .getChildFile("Factory Presets");

    // Ensure directory exists
    currentPresetDirectory.createDirectory();
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
    // prevPresetButton.setBounds(10, buttonsY, buttonWidth, 30);
    // nextPresetButton.setBounds(buttonWidth + buttonSpacing + 10, buttonsY, buttonWidth, 30);
    
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
                // updatePresetList();
            }
            dialogWindow.reset();  // Add this line to clean up
        }
    ));
}

void PluginEditor::loadPreset()
{
    // Create file browser component
    auto tempBrowser = std::make_unique<juce::FileBrowserComponent>(
        juce::FileBrowserComponent::openMode | 
        juce::FileBrowserComponent::canSelectFiles,
        currentPresetDirectory,  // Start in presets directory
        new juce::WildcardFileFilter("*.preset", "*", "Preset Files"),
        nullptr);

    // Style and add listener before transferring ownership
    tempBrowser->setLookAndFeel(&getLookAndFeel());
    tempBrowser->addListener(this);

    // Create dialog window
    fileBrowserDialog = std::make_unique<juce::DialogWindow>(
        "Load Preset",
        juce::Colour(0xFF191919),
        true,
        true);

    // Store pointer before transferring ownership
    fileBrowser = tempBrowser.get();
    
    // Transfer ownership
    fileBrowserDialog->setContentOwned(tempBrowser.release(), true);
    fileBrowserDialog->centreAroundComponent(this, 400, 300);

    fileBrowserDialog->enterModalState(true,
        juce::ModalCallbackFunction::create([this](int result) {
            if (result == 1)  // OK was pressed
            {
                if (fileBrowser != nullptr)
                {
                    juce::File selectedFile = fileBrowser->getSelectedFile(true);
                    if (selectedFile.existsAsFile())
                    {
                        auto data = PresetData::loadFromFile(selectedFile);
                        harm1.setHarmonicData(data.harm1Data);
                        harm2.setHarmonicData(data.harm2Data);
                        combo.setHarmonicData(data.comboData);
                        morphSlider.setValue(data.morphValue, juce::sendNotification);
                        
                        // Store in processor
                        processorRef.setHarmonicData(
                            data.harm1Data,
                            data.harm2Data,
                            data.comboData
                        );
                    }
                }
            }
            fileBrowser = nullptr;  // Clear the raw pointer
            fileBrowserDialog = nullptr;
        }));
}

juce::Array<float> PluginEditor::getComboHarmonicData() const
{
    return combo.getHarmonicData();
}