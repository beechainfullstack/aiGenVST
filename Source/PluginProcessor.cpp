#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AIGenVSTProcessor::AIGenVSTProcessor()
     : AudioProcessor (BusesProperties()
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
}

AIGenVSTProcessor::~AIGenVSTProcessor()
{
    if (generationThread != nullptr)
    {
        generationThread->stopThread(5000);
        generationThread.reset();
    }
}

//==============================================================================
const juce::String AIGenVSTProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AIGenVSTProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AIGenVSTProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

int AIGenVSTProcessor::getTailLengthSeconds() const
{
    return 0;
}

int AIGenVSTProcessor::getNumPrograms()
{
    return 1;
}

int AIGenVSTProcessor::getCurrentProgram()
{
    return 0;
}

void AIGenVSTProcessor::setCurrentProgram (int index)
{
}

const juce::String AIGenVSTProcessor::getProgramName (int index)
{
    return {};
}

void AIGenVSTProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AIGenVSTProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sampler.setCurrentPlaybackSampleRate(sampleRate);
}

void AIGenVSTProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AIGenVSTProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}
#endif

void AIGenVSTProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    // Clear output buffer
    buffer.clear();
    
    // Render sampler
    sampler.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

//==============================================================================
bool AIGenVSTProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* AIGenVSTProcessor::createEditor()
{
    return new AIGenVSTEditor (*this);
}

//==============================================================================
void AIGenVSTProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save plugin state (could save last prompt, sample data, etc.)
    // For MVP, we'll skip this
}

void AIGenVSTProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore plugin state
    // For MVP, we'll skip this
}

//==============================================================================
void AIGenVSTProcessor::generateInstrumentFromPrompt(const juce::String& prompt, float duration)
{
    if (generating.load())
    {
        DBG("Generation already in progress");
        return;
    }
    
    // Stop any existing thread
    if (generationThread != nullptr)
    {
        generationThread->stopThread(1000);
        generationThread.reset();
    }
    
    // Start new generation thread
    generating.store(true);
    generationStatus = "Starting generation...";
    
    generationThread = std::make_unique<GenerationThread>(*this, prompt, duration);
    generationThread->startThread();
}

void AIGenVSTProcessor::runGeneration(const juce::String& prompt, float duration)
{
    try
    {
        generationStatus = "Calling AI model...";
        
        // Call AI generator
        auto result = aiGenerator.generate(prompt, duration);
        
        if (result.success)
        {
            generationStatus = "Processing audio...";
            
            // Load the generated WAV into sampler
            sampler.loadSampleFromFile(result.wavFilePath);
            
            generationStatus = "Ready! Play MIDI notes.";
            
            // Clean up temp file (optional)
            // juce::File(result.wavFilePath).deleteFile();
        }
        else
        {
            generationStatus = "Error: " + result.errorMessage;
            DBG("Generation failed: " + result.errorMessage);
        }
    }
    catch (const std::exception& e)
    {
        generationStatus = "Exception: " + juce::String(e.what());
        DBG("Exception during generation: " + juce::String(e.what()));
    }
    
    generating.store(false);
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AIGenVSTProcessor();
}
