#pragma once
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>

struct PresetData
{
    juce::Array<float> harm1Data;
    juce::Array<float> harm2Data;
    juce::Array<float> comboData;
    float morphValue;

    void saveToFile(const juce::File& file) const
    {
        juce::ValueTree preset("PRESET");
        
        // Save harm1 data
        juce::ValueTree harm1Tree("HARM1");
        for (int i = 0; i < harm1Data.size(); ++i)
            harm1Tree.setProperty("h" + juce::String(i), harm1Data[i], nullptr);
        
        // Save harm2 data
        juce::ValueTree harm2Tree("HARM2");
        for (int i = 0; i < harm2Data.size(); ++i)
            harm2Tree.setProperty("h" + juce::String(i), harm2Data[i], nullptr);

        // Save combo data
        juce::ValueTree comboTree("COMBO");
        for (int i = 0; i < comboData.size(); ++i)
            comboTree.setProperty("h" + juce::String(i), comboData[i], nullptr);
        
        // Save morph value
        preset.setProperty("morphValue", morphValue, nullptr);
        
        preset.addChild(harm1Tree, -1, nullptr);
        preset.addChild(harm2Tree, -1, nullptr);
        preset.addChild(comboTree, -1, nullptr);
        
        if (auto xml = preset.createXml())
            xml->writeTo(file);
    }

    static PresetData loadFromFile(const juce::File& file)
    {
        PresetData data;
        
        if (auto xml = juce::XmlDocument::parse(file))
        {
            auto preset = juce::ValueTree::fromXml(*xml);
            
            auto harm1Tree = preset.getChildWithName("HARM1");
            if (harm1Tree.isValid())
            {
                for (int i = 0; i < 8; ++i)
                {
                    auto value = harm1Tree.getProperty("h" + juce::String(i));
                    data.harm1Data.add(value);
                }
            }
            
            auto harm2Tree = preset.getChildWithName("HARM2");
            if (harm2Tree.isValid())
            {
                for (int i = 0; i < 8; ++i)
                {
                    auto value = harm2Tree.getProperty("h" + juce::String(i));
                    data.harm2Data.add(value);
                }
            }

            auto comboTree = preset.getChildWithName("COMBO");
            if (comboTree.isValid())
            {
                for (int i = 0; i < 8; ++i)
                {
                    auto value = comboTree.getProperty("h" + juce::String(i));
                    data.comboData.add(value);
                }
            }
            
            data.morphValue = preset.getProperty("morphValue", 0.0f);
        }
        
        return data;
    }
};