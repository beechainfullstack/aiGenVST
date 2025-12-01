#pragma once

#include <JuceHeader.h>

//==============================================================================
// Custom sampler sound that stores our generated audio
class AISamplerSound : public juce::SynthesiserSound
{
public:
    AISamplerSound(const juce::String& name,
                   juce::AudioBuffer<float>& source,
                   int rootMidiNote,
                   double sampleRate);
    
    bool appliesToNote(int midiNoteNumber) override { return true; }
    bool appliesToChannel(int midiChannel) override { return true; }
    
    const juce::AudioBuffer<float>& getAudioData() const { return audioData; }
    int getRootNote() const { return rootNote; }
    double getSourceSampleRate() const { return sourceSampleRate; }
    int getLoopStart() const { return loopStart; }
    int getLoopEnd() const { return loopEnd; }
    int getLength() const { return audioData.getNumSamples(); }
    
    void setLoopPoints(int start, int end)
    {
        loopStart = start;
        loopEnd = end;
    }

private:
    juce::AudioBuffer<float> audioData;
    int rootNote;
    double sourceSampleRate;
    int loopStart = 0;
    int loopEnd = 0;
};

//==============================================================================
// Custom sampler voice that plays back with pitch shifting
class AISamplerVoice : public juce::SynthesiserVoice
{
public:
    AISamplerVoice();
    
    bool canPlaySound(juce::SynthesiserSound* sound) override;
    
    void startNote(int midiNoteNumber, float velocity,
                   juce::SynthesiserSound* sound,
                   int currentPitchWheelPosition) override;
    
    void stopNote(float velocity, bool allowTailOff) override;
    
    void pitchWheelMoved(int newPitchWheelValue) override {}
    void controllerMoved(int controllerNumber, int newControllerValue) override {}
    
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                         int startSample, int numSamples) override;

private:
    double pitchRatio = 1.0;
    double sourceSamplePosition = 0.0;
    float currentVelocity = 0.0f;
    
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;
    
    void updatePitchRatio(int midiNote, AISamplerSound* sound);
};

//==============================================================================
// Main sampler engine
class AISamplerEngine : public juce::Synthesiser
{
public:
    AISamplerEngine();
    
    void loadSampleFromFile(const juce::String& filePath);
    void loadSampleFromBuffer(juce::AudioBuffer<float>& buffer, int rootNote = 60);
    
    bool hasSampleLoaded() const { return sampleLoaded; }
    juce::String getLoadedSampleInfo() const { return sampleInfo; }

private:
    bool sampleLoaded = false;
    juce::String sampleInfo;
    
    static constexpr int maxVoices = 16;
    
    void processLoadedBuffer(juce::AudioBuffer<float>& buffer, double sampleRate);
    void trimSilence(juce::AudioBuffer<float>& buffer);
    void normalize(juce::AudioBuffer<float>& buffer, float targetDB = -0.5f);
    int detectPitch(const juce::AudioBuffer<float>& buffer, double sampleRate);
    void findLoopPoints(const juce::AudioBuffer<float>& buffer, int& loopStart, int& loopEnd);
};
