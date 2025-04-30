#pragma once

#include "PluginProcessor.h"
#include "BinaryData.h"
#include "melatonin_inspector/melatonin_inspector.h"
#include "Harm.h"
#include "Preset.h"

class PluginEditor : public juce::AudioProcessorEditor,
                    public juce::FileBrowserListener
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    // Override methods
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void selectionChanged() override {}
    void fileClicked(const juce::File&, const juce::MouseEvent&) override {}
    void fileDoubleClicked(const juce::File&) override {}
    void browserRootChanged(const juce::File&) override {}

    juce::Array<float> getComboHarmonicData() const;

private:
    // Member functions
    void savePreset();
    void loadPreset();

    // Member variables
    PluginProcessor& processorRef;
    std::unique_ptr<melatonin::Inspector> inspector;
    juce::TextButton inspectButton { "Inspect the UI" };
    juce::TextButton savePresetButton { "Save Preset" };
    juce::TextButton loadPresetButton { "Load Preset" };
    std::unique_ptr<juce::Drawable> background;
    Harm harm1 { juce::Colour(0xffc7884d) };
    Harm combo { juce::Colour(0xffE0E0E0) };
    Harm harm2 { juce::Colour(0xff89b4c1) };
    juce::Slider morphSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> morphAttachment;
    juce::ResizableCornerComponent resizer;
    juce::ComponentBoundsConstrainer constrainer;

    std::unique_ptr<juce::DialogWindow> fileBrowserDialog;
    juce::FileBrowserComponent* fileBrowser = nullptr;
    juce::File currentPresetDirectory;

    // Alert window for save dialog
    std::unique_ptr<juce::AlertWindow> dialogWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};

