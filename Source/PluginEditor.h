#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class AIGenVSTEditor : public juce::AudioProcessorEditor,
                       private juce::Timer
{
public:
    AIGenVSTEditor (AIGenVSTProcessor&);
    ~AIGenVSTEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void generateButtonClicked();
    
    AIGenVSTProcessor& audioProcessor;
    
    // UI Components
    juce::Label titleLabel;
    juce::Label promptLabel;
    juce::TextEditor promptInput;
    juce::TextButton generateButton;
    juce::Label statusLabel;
    juce::Label infoLabel;
    
    // Styling
    juce::Colour backgroundColour = juce::Colour(0xff1a1a1a);
    juce::Colour accentColour = juce::Colour(0xff4CAF50);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AIGenVSTEditor)
};
