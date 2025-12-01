#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AIGenVSTEditor::AIGenVSTEditor (AIGenVSTProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (500, 300);
    
    // Title Label
    titleLabel.setText("AI Instrument Generator", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);
    
    // Prompt Label
    promptLabel.setText("Enter your prompt:", juce::dontSendNotification);
    promptLabel.setFont(juce::Font(14.0f));
    promptLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(promptLabel);
    
    // Prompt Input
    promptInput.setMultiLine(false);
    promptInput.setReturnKeyStartsNewLine(false);
    promptInput.setText("deep bass synth");
    promptInput.setFont(juce::Font(14.0f));
    promptInput.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff2a2a2a));
    promptInput.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    promptInput.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff3a3a3a));
    addAndMakeVisible(promptInput);
    
    // Generate Button
    generateButton.setButtonText("Generate Instrument");
    generateButton.setColour(juce::TextButton::buttonColourId, accentColour);
    generateButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    generateButton.onClick = [this] { generateButtonClicked(); };
    addAndMakeVisible(generateButton);
    
    // Status Label
    statusLabel.setText("Ready", juce::dontSendNotification);
    statusLabel.setFont(juce::Font(12.0f));
    statusLabel.setJustificationType(juce::Justification::centred);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(statusLabel);
    
    // Info Label
    infoLabel.setText("No sample loaded", juce::dontSendNotification);
    infoLabel.setFont(juce::Font(11.0f, juce::Font::italic));
    infoLabel.setJustificationType(juce::Justification::centred);
    infoLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
    addAndMakeVisible(infoLabel);
    
    // Start timer for status updates
    startTimer(100); // Update 10 times per second
}

AIGenVSTEditor::~AIGenVSTEditor()
{
    stopTimer();
}

//==============================================================================
void AIGenVSTEditor::paint (juce::Graphics& g)
{
    g.fillAll(backgroundColour);
    
    // Draw a subtle border
    g.setColour(juce::Colour(0xff3a3a3a));
    g.drawRect(getLocalBounds(), 2);
}

void AIGenVSTEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    
    titleLabel.setBounds(area.removeFromTop(40));
    area.removeFromTop(10);
    
    promptLabel.setBounds(area.removeFromTop(20));
    area.removeFromTop(5);
    
    promptInput.setBounds(area.removeFromTop(30));
    area.removeFromTop(15);
    
    generateButton.setBounds(area.removeFromTop(40));
    area.removeFromTop(15);
    
    statusLabel.setBounds(area.removeFromTop(25));
    area.removeFromTop(5);
    
    infoLabel.setBounds(area.removeFromTop(20));
}

void AIGenVSTEditor::timerCallback()
{
    // Update status from processor
    statusLabel.setText(audioProcessor.getGenerationStatus(), juce::dontSendNotification);
    
    // Update button state
    generateButton.setEnabled(!audioProcessor.isGenerating());
    
    // Update info label
    if (audioProcessor.getSampler().hasSampleLoaded())
    {
        infoLabel.setText(audioProcessor.getSampler().getLoadedSampleInfo(), juce::dontSendNotification);
        infoLabel.setColour(juce::Label::textColourId, accentColour);
    }
    
    // Change button color when generating
    if (audioProcessor.isGenerating())
    {
        generateButton.setColour(juce::TextButton::buttonColourId, juce::Colours::orange);
    }
    else
    {
        generateButton.setColour(juce::TextButton::buttonColourId, accentColour);
    }
}

void AIGenVSTEditor::generateButtonClicked()
{
    juce::String prompt = promptInput.getText();
    
    if (prompt.isEmpty())
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "Empty Prompt",
            "Please enter a description for the instrument you want to generate.",
            "OK"
        );
        return;
    }
    
    // Trigger generation in processor
    audioProcessor.generateInstrumentFromPrompt(prompt, 3.0f);
}
