#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginProcessor::PluginProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
{
    // Initialize harmonic data with default values
    harm1Data.resize(8);
    harm2Data.resize(8);
    comboData.resize(8);
    
    // Set some default values (for example)
    for (int i = 0; i < 8; ++i)
    {
        harm1Data.set(i, 0.0f);
        harm2Data.set(i, 0.0f);
        comboData.set(i, 0.0f);
    }
}

PluginProcessor::~PluginProcessor()
{
}

//==============================================================================
const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String PluginProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void PluginProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void PluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                 juce::MidiBuffer& midiMessages)
{
    // Helper function to convert frequency ratio to semitones
    auto ratioToSemitones = [](float ratio) {
        return static_cast<int>(std::round(12.0f * std::log2(ratio)));
    };

    juce::MidiBuffer processedMidi;
    
    for (const auto metadata : midiMessages)
    {
        const auto message = metadata.getMessage();
        const auto time = metadata.samplePosition;

        if (message.isNoteOn())
        {
            const int baseNote = message.getNoteNumber();
            const int baseVelocity = message.getVelocity();
            
            processedMidi.addEvent(message, time);
            
            // Use stored data directly instead of accessing editor
            auto& harmonicData = comboData;
            
            for (int i = 0; i < harmonicData.size(); ++i)
            {
                float harmonicStrength = harmonicData[i];
                if (harmonicStrength > 0.0f)
                {
                    // Calculate the frequency ratio for this harmonic
                    // Harmonic series is 1:2:3:4:5:6:7:8
                    float ratio = static_cast<float>(i + 2); // +2 because i starts at 0
                    int harmonicNote = baseNote + ratioToSemitones(ratio);
                    
                    int harmonicVelocity = juce::jlimit(1, 127, 
                        static_cast<int>(baseVelocity * harmonicStrength));
                    
                    if (harmonicNote <= 127)
                    {
                        processedMidi.addEvent(
                            juce::MidiMessage::noteOn(message.getChannel(), 
                                                        harmonicNote, 
                                                        static_cast<uint8_t>(harmonicVelocity)),
                            time);
                    }
                }
            }
        }
        else if (message.isNoteOff())
        {
            const int baseNote = message.getNoteNumber();
            processedMidi.addEvent(message, time);
            
            // Send note-offs using the same ratio calculation
            for (int i = 0; i < 8; ++i)
            {
                float ratio = static_cast<float>(i + 2);
                int harmonicNote = baseNote + ratioToSemitones(ratio);
                if (harmonicNote <= 127)
                {
                    processedMidi.addEvent(
                        juce::MidiMessage::noteOff(message.getChannel(), harmonicNote),
                        time);
                }
            }
        }
        else
        {
            processedMidi.addEvent(message, time);
        }
    }
    
    midiMessages.swapWith(processedMidi);

    // Clear audio outputs
    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Get automatable parameters state from APVTS
    auto state = apvts.copyState();
    
    // Add non-automatable harmonic table data as custom XML
    auto* harmonicsXml = new juce::XmlElement("HarmonicData");
    
    // Save harm1 data
    auto* harm1Xml = new juce::XmlElement("Harm1");
    for (int i = 0; i < harm1Data.size(); ++i)
        harm1Xml->setAttribute("h" + juce::String(i), harm1Data[i]);
    harmonicsXml->addChildElement(harm1Xml);
    
    // Save harm2 data
    auto* harm2Xml = new juce::XmlElement("Harm2");
    for (int i = 0; i < harm2Data.size(); ++i)
        harm2Xml->setAttribute("h" + juce::String(i), harm2Data[i]);
    harmonicsXml->addChildElement(harm2Xml);
    
    // Save combo data
    auto* comboXml = new juce::XmlElement("Combo");
    for (int i = 0; i < comboData.size(); ++i)
        comboXml->setAttribute("h" + juce::String(i), comboData[i]);
    harmonicsXml->addChildElement(comboXml);
    
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    xml->addChildElement(harmonicsXml);
    copyXmlToBinary(*xml, destData);
}

void PluginProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName(apvts.state.getType()))
        {
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
            
            // Load harmonic data
            if (auto* harmonicsXml = xmlState->getChildByName("HarmonicData"))
            {
                if (auto* harm1Xml = harmonicsXml->getChildByName("Harm1"))
                    for (int i = 0; i < 8; ++i)
                        harm1Data.set(i, static_cast<float>(harm1Xml->getDoubleAttribute("h" + juce::String(i), 0.0)));
                
                if (auto* harm2Xml = harmonicsXml->getChildByName("Harm2"))
                    for (int i = 0; i < 8; ++i)
                        harm2Data.set(i, static_cast<float>(harm2Xml->getDoubleAttribute("h" + juce::String(i), 0.0)));
                
                if (auto* comboXml = harmonicsXml->getChildByName("Combo"))
                    for (int i = 0; i < 8; ++i)
                        comboData.set(i, static_cast<float>(comboXml->getDoubleAttribute("h" + juce::String(i), 0.0)));
            }
        }
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("Morph", 1),    // parameter ID with version hint of 1
        "Morph",    // parameter name
        0.0f,       // minimum value
        1.0f,       // maximum value
        0.5f        // default value
    ));
    
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}