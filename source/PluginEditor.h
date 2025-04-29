#pragma once

#include "PluginProcessor.h"
#include "BinaryData.h"
#include "melatonin_inspector/melatonin_inspector.h"
#include "Harm.h"
#include "Preset.h"

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (PluginProcessor&);
    juce::Array<float> getComboHarmonicData() const;  
    ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginProcessor& processorRef;
    std::unique_ptr<melatonin::Inspector> inspector;
    juce::TextButton inspectButton { "Inspect the UI" };
    juce::TextButton savePresetButton { "Save Preset" };
    void savePreset();
    juce::TextButton loadPresetButton { "Load Preset" };
    void loadPreset();
    std::unique_ptr<juce::Drawable> background;
    Harm harm1 { juce::Colour(0xffc7884d) };
    Harm combo { juce::Colour(0xffE0E0E0) };
    Harm harm2 { juce::Colour(0xff89b4c1) };
    juce::Slider morphSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> morphAttachment;
    juce::ResizableCornerComponent resizer;
    juce::ComponentBoundsConstrainer constrainer;
    std::unique_ptr<juce::FileChooser> fileChooser;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)

private:
    // Add these members
    juce::TextButton nextPresetButton { "Next" };
    juce::TextButton prevPresetButton { "Prev" };
    juce::File currentPresetDirectory;
    juce::Array<juce::File> presetFiles;
    int currentPresetIndex = -1;
    
    void loadNextPreset();
    void loadPrevPreset();
    void updatePresetList();
    void loadPresetAtIndex(int index);
    std::unique_ptr<juce::AlertWindow> dialogWindow;  // Add this line
};

