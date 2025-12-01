#pragma once

#include <JuceHeader.h>
#include "SamplerEngine.h"
#include "AIGenerator.h"

//==============================================================================
class AIGenVSTProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    AIGenVSTProcessor();
    ~AIGenVSTProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    int getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Custom methods for AI generation
    void generateInstrumentFromPrompt(const juce::String& prompt, float duration = 3.0f);
    bool isGenerating() const { return generating.load(); }
    juce::String getGenerationStatus() const { return generationStatus; }
    
    // Access to sampler for UI
    AISamplerEngine& getSampler() { return sampler; }

private:
    //==============================================================================
    AISamplerEngine sampler;
    AIGenerator aiGenerator;
    
    std::atomic<bool> generating { false };
    juce::String generationStatus;
    
    // Background thread for AI generation
    class GenerationThread : public juce::Thread
    {
    public:
        GenerationThread(AIGenVSTProcessor& p, const juce::String& prompt, float duration)
            : Thread("AI Generation"), processor(p), promptText(prompt), audioDuration(duration)
        {}
        
        void run() override
        {
            processor.runGeneration(promptText, audioDuration);
        }
        
    private:
        AIGenVSTProcessor& processor;
        juce::String promptText;
        float audioDuration;
    };
    
    std::unique_ptr<GenerationThread> generationThread;
    
    void runGeneration(const juce::String& prompt, float duration);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AIGenVSTProcessor)
};
