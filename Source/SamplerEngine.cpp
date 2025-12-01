#include "SamplerEngine.h"
#include "PitchDetector.h"

//==============================================================================
// AISamplerSound Implementation
//==============================================================================
AISamplerSound::AISamplerSound(const juce::String& name,
                               juce::AudioBuffer<float>& source,
                               int rootMidiNote,
                               double sampleRate)
    : rootNote(rootMidiNote), sourceSampleRate(sampleRate)
{
    audioData.makeCopyOf(source);
    loopEnd = audioData.getNumSamples();
}

//==============================================================================
// AISamplerVoice Implementation
//==============================================================================
AISamplerVoice::AISamplerVoice()
{
    // Default ADSR: quick attack, medium release
    adsrParams.attack = 0.01f;
    adsrParams.decay = 0.1f;
    adsrParams.sustain = 0.8f;
    adsrParams.release = 0.3f;
    adsr.setParameters(adsrParams);
}

bool AISamplerVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<AISamplerSound*>(sound) != nullptr;
}

void AISamplerVoice::startNote(int midiNoteNumber, float velocity,
                                juce::SynthesiserSound* sound,
                                int currentPitchWheelPosition)
{
    if (auto* samplerSound = dynamic_cast<AISamplerSound*>(sound))
    {
        currentVelocity = velocity;
        sourceSamplePosition = 0.0;
        
        updatePitchRatio(midiNoteNumber, samplerSound);
        
        adsr.setSampleRate(samplerSound->getSourceSampleRate());
        adsr.noteOn();
    }
}

void AISamplerVoice::stopNote(float velocity, bool allowTailOff)
{
    if (allowTailOff)
    {
        adsr.noteOff();
    }
    else
    {
        clearCurrentNote();
        adsr.reset();
    }
}

void AISamplerVoice::updatePitchRatio(int midiNote, AISamplerSound* sound)
{
    // Calculate pitch ratio for resampling
    // Each semitone is 2^(1/12) frequency ratio
    int semitoneOffset = midiNote - sound->getRootNote();
    pitchRatio = std::pow(2.0, semitoneOffset / 12.0);
}

void AISamplerVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                                      int startSample, int numSamples)
{
    if (auto* samplerSound = dynamic_cast<AISamplerSound*>(getCurrentlyPlayingSound().get()))
    {
        const auto& sourceBuffer = samplerSound->getAudioData();
        const int sourceLength = sourceBuffer.getNumSamples();
        const int loopStart = samplerSound->getLoopStart();
        const int loopEnd = samplerSound->getLoopEnd();
        const bool looping = (loopEnd > loopStart) && (loopEnd <= sourceLength);
        
        for (int i = 0; i < numSamples; ++i)
        {
            // Check if we're done playing
            if (!adsr.isActive() || (sourceSamplePosition >= sourceLength && !looping))
            {
                clearCurrentNote();
                break;
            }
            
            // Handle looping
            if (looping && sourceSamplePosition >= loopEnd)
            {
                sourceSamplePosition = loopStart + (sourceSamplePosition - loopEnd);
            }
            
            // Linear interpolation for resampling
            int index0 = (int)sourceSamplePosition;
            int index1 = index0 + 1;
            
            if (index1 >= sourceLength)
            {
                if (!looping)
                    break;
                index1 = loopStart;
            }
            
            float frac = (float)(sourceSamplePosition - index0);
            float sample0 = sourceBuffer.getSample(0, index0);
            float sample1 = sourceBuffer.getSample(0, index1);
            float interpolatedSample = sample0 + frac * (sample1 - sample0);
            
            // Apply ADSR envelope and velocity
            float envelope = adsr.getNextSample();
            float outputSample = interpolatedSample * envelope * currentVelocity;
            
            // Write to output (stereo)
            for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
            {
                outputBuffer.addSample(channel, startSample + i, outputSample);
            }
            
            // Advance playback position with pitch shifting
            sourceSamplePosition += pitchRatio;
        }
    }
}

//==============================================================================
// AISamplerEngine Implementation
//==============================================================================
AISamplerEngine::AISamplerEngine()
{
    // Add voices
    for (int i = 0; i < maxVoices; ++i)
        addVoice(new AISamplerVoice());
}

void AISamplerEngine::loadSampleFromFile(const juce::String& filePath)
{
    juce::File audioFile(filePath);
    
    if (!audioFile.existsAsFile())
    {
        DBG("File does not exist: " + filePath);
        return;
    }
    
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(audioFile));
    
    if (reader == nullptr)
    {
        DBG("Failed to create reader for: " + filePath);
        return;
    }
    
    juce::AudioBuffer<float> buffer((int)reader->numChannels, (int)reader->lengthInSamples);
    reader->read(&buffer, 0, (int)reader->lengthInSamples, 0, true, true);
    
    // Convert to mono if stereo
    if (buffer.getNumChannels() > 1)
    {
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float mono = 0.0f;
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                mono += buffer.getSample(ch, i);
            mono /= buffer.getNumChannels();
            buffer.setSample(0, i, mono);
        }
        buffer.setSize(1, buffer.getNumSamples(), true);
    }
    
    processLoadedBuffer(buffer, reader->sampleRate);
}

void AISamplerEngine::loadSampleFromBuffer(juce::AudioBuffer<float>& buffer, int rootNote)
{
    processLoadedBuffer(buffer, 44100.0); // Assume 44.1kHz for now
}

void AISamplerEngine::processLoadedBuffer(juce::AudioBuffer<float>& buffer, double sampleRate)
{
    // Step 1: Trim silence
    trimSilence(buffer);
    
    // Step 2: Normalize
    normalize(buffer);
    
    // Step 3: Detect pitch
    int rootNote = detectPitch(buffer, sampleRate);
    
    // Step 4: Find loop points
    int loopStart = 0;
    int loopEnd = buffer.getNumSamples();
    findLoopPoints(buffer, loopStart, loopEnd);
    
    // Step 5: Create sampler sound and load it
    clearSounds();
    auto* sound = new AISamplerSound("Generated", buffer, rootNote, sampleRate);
    sound->setLoopPoints(loopStart, loopEnd);
    addSound(sound);
    
    sampleLoaded = true;
    sampleInfo = juce::String::formatted("Root: %d, Length: %.2fs, Loop: %d-%d",
                                         rootNote,
                                         buffer.getNumSamples() / sampleRate,
                                         loopStart, loopEnd);
    
    DBG("Sample loaded: " + sampleInfo);
}

void AISamplerEngine::trimSilence(juce::AudioBuffer<float>& buffer)
{
    const float threshold = 0.001f; // -60 dB roughly
    int startSample = 0;
    int endSample = buffer.getNumSamples() - 1;
    
    // Find first non-silent sample
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        if (std::abs(buffer.getSample(0, i)) > threshold)
        {
            startSample = i;
            break;
        }
    }
    
    // Find last non-silent sample
    for (int i = buffer.getNumSamples() - 1; i >= 0; --i)
    {
        if (std::abs(buffer.getSample(0, i)) > threshold)
        {
            endSample = i;
            break;
        }
    }
    
    // Trim the buffer
    if (endSample > startSample)
    {
        int newLength = endSample - startSample + 1;
        juce::AudioBuffer<float> trimmedBuffer(1, newLength);
        
        for (int i = 0; i < newLength; ++i)
            trimmedBuffer.setSample(0, i, buffer.getSample(0, startSample + i));
        
        buffer = std::move(trimmedBuffer);
    }
}

void AISamplerEngine::normalize(juce::AudioBuffer<float>& buffer, float targetDB)
{
    // Find peak
    float peak = 0.0f;
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        float sample = std::abs(buffer.getSample(0, i));
        if (sample > peak)
            peak = sample;
    }
    
    if (peak > 0.0f)
    {
        // Convert target dB to linear gain
        float targetLinear = juce::Decibels::decibelsToGain(targetDB);
        float gain = targetLinear / peak;
        
        // Apply gain
        buffer.applyGain(gain);
    }
}

int AISamplerEngine::detectPitch(const juce::AudioBuffer<float>& buffer, double sampleRate)
{
    // Use autocorrelation-based pitch detection
    PitchDetector detector;
    float frequency = detector.detectPitch(buffer, sampleRate);
    
    if (frequency > 0.0f)
    {
        // Convert frequency to MIDI note
        // A4 (MIDI 69) = 440 Hz
        // MIDI note = 69 + 12 * log2(freq / 440)
        int midiNote = 69 + (int)(12.0f * std::log2(frequency / 440.0f));
        
        // Clamp to reasonable range
        midiNote = juce::jlimit(0, 127, midiNote);
        
        return midiNote;
    }
    
    // Default to C3 if detection fails
    return 60;
}

void AISamplerEngine::findLoopPoints(const juce::AudioBuffer<float>& buffer, int& loopStart, int& loopEnd)
{
    // For MVP: Loop the last 75% of the sample
    // In production, find zero crossings for seamless loops
    int totalLength = buffer.getNumSamples();
    loopStart = totalLength / 4;
    loopEnd = totalLength;
    
    // Find zero crossing near loop points for smoother loops
    for (int i = loopStart; i < loopStart + 1000 && i < totalLength - 1; ++i)
    {
        if (buffer.getSample(0, i) * buffer.getSample(0, i + 1) <= 0.0f)
        {
            loopStart = i;
            break;
        }
    }
}
