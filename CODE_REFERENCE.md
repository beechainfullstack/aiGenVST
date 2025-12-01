# Code Reference - Key Snippets

Quick reference for the most important code patterns in this project.

## Plugin Basics

### Creating a JUCE Plugin
```cpp
class AIGenVSTProcessor : public juce::AudioProcessor
{
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
};
```

### Audio Processing Loop
```cpp
void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    buffer.clear();  // Clear output
    sampler.renderNextBlock(buffer, midi, 0, buffer.getNumSamples());
}
```

## Sampler Engine

### Custom Sampler Voice
```cpp
class AISamplerVoice : public juce::SynthesiserVoice
{
    void startNote(int midiNote, float velocity, 
                   juce::SynthesiserSound* sound, int pitchWheel) override
    {
        // Calculate pitch shift ratio
        int semitoneOffset = midiNote - samplerSound->getRootNote();
        pitchRatio = std::pow(2.0, semitoneOffset / 12.0);
        
        adsr.noteOn();
    }
    
    void renderNextBlock(juce::AudioBuffer<float>& output, 
                         int start, int num) override
    {
        for (int i = 0; i < num; ++i)
        {
            // Linear interpolation for pitch shifting
            int idx0 = (int)samplePosition;
            int idx1 = idx0 + 1;
            float frac = samplePosition - idx0;
            
            float sample = lerp(buffer[idx0], buffer[idx1], frac);
            sample *= adsr.getNextSample() * velocity;
            
            output.addSample(channel, start + i, sample);
            samplePosition += pitchRatio;
        }
    }
};
```

## Pitch Detection

### Autocorrelation Method
```cpp
float detectPitch(const juce::AudioBuffer<float>& buffer, double sampleRate)
{
    float maxCorrelation = -1.0f;
    int bestLag = 0;
    
    // Try different lags (periods)
    for (int lag = minPeriod; lag < maxPeriod; ++lag)
    {
        float correlation = 0.0f;
        
        // Autocorrelation at this lag
        for (int i = 0; i < length - lag; ++i)
            correlation += buffer[i] * buffer[i + lag];
        
        if (correlation > maxCorrelation)
        {
            maxCorrelation = correlation;
            bestLag = lag;
        }
    }
    
    // Convert lag to frequency
    return sampleRate / bestLag;
}
```

### Frequency to MIDI Note
```cpp
int frequencyToMIDI(float frequency)
{
    // A4 (MIDI 69) = 440 Hz
    // MIDI = 69 + 12 * log2(freq / 440)
    return 69 + (int)(12.0f * std::log2(frequency / 440.0f));
}
```

## Audio Processing

### Trim Silence
```cpp
void trimSilence(juce::AudioBuffer<float>& buffer, float threshold = 0.001f)
{
    int start = 0, end = buffer.getNumSamples() - 1;
    
    // Find first non-silent sample
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        if (std::abs(buffer.getSample(0, i)) > threshold)
        {
            start = i;
            break;
        }
    }
    
    // Find last non-silent sample
    for (int i = buffer.getNumSamples() - 1; i >= 0; --i)
    {
        if (std::abs(buffer.getSample(0, i)) > threshold)
        {
            end = i;
            break;
        }
    }
    
    // Extract trimmed portion
    int newLength = end - start + 1;
    juce::AudioBuffer<float> trimmed(1, newLength);
    for (int i = 0; i < newLength; ++i)
        trimmed.setSample(0, i, buffer.getSample(0, start + i));
    
    buffer = std::move(trimmed);
}
```

### Normalize Audio
```cpp
void normalize(juce::AudioBuffer<float>& buffer, float targetDB = -0.5f)
{
    // Find peak
    float peak = 0.0f;
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        float sample = std::abs(buffer.getSample(0, i));
        peak = std::max(peak, sample);
    }
    
    if (peak > 0.0f)
    {
        float targetLinear = juce::Decibels::decibelsToGain(targetDB);
        float gain = targetLinear / peak;
        buffer.applyGain(gain);
    }
}
```

### Find Zero Crossings
```cpp
int findNextZeroCrossing(const juce::AudioBuffer<float>& buffer, int startPos)
{
    for (int i = startPos; i < buffer.getNumSamples() - 1; ++i)
    {
        float current = buffer.getSample(0, i);
        float next = buffer.getSample(0, i + 1);
        
        // Sign change = zero crossing
        if (current * next <= 0.0f)
            return i;
    }
    return startPos;
}
```

## HTTP Communication

### JSON Request (C++)
```cpp
juce::var sendJSONRequest(const juce::String& url, const juce::String& prompt)
{
    // Build JSON
    juce::DynamicObject::Ptr json = new juce::DynamicObject();
    json->setProperty("prompt", prompt);
    json->setProperty("duration", 3.0);
    
    juce::String jsonString = juce::JSON::toString(json);
    
    // Send POST
    juce::URL endpoint(url);
    auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
        .withConnectionTimeoutMs(60000)
        .withExtraHeaders("Content-Type: application/json")
        .withHttpRequestCmd("POST")
        .withPostData(jsonString);
    
    std::unique_ptr<juce::InputStream> stream(endpoint.createInputStream(options));
    
    if (stream != nullptr)
    {
        juce::String response = stream->readEntireStreamAsString();
        juce::var parsed;
        juce::JSON::parse(response, parsed);
        return parsed;
    }
    
    return {};
}
```

### Flask Server (Python)
```python
from flask import Flask, request, jsonify

app = Flask(__name__)

@app.route('/generate', methods=['POST'])
def generate():
    data = request.get_json()
    prompt = data['prompt']
    duration = data['duration']
    
    # Generate audio
    wav_path = generate_audio(prompt, duration)
    
    return jsonify({
        'wav_path': wav_path,
        'success': True
    })

app.run(host='0.0.0.0', port=5000)
```

## UI Components

### Text Input
```cpp
juce::TextEditor promptInput;

promptInput.setMultiLine(false);
promptInput.setText("deep bass synth");
promptInput.onReturnKey = [this] { 
    generateButtonClicked(); 
};
addAndMakeVisible(promptInput);
```

### Button with Callback
```cpp
juce::TextButton generateButton;

generateButton.setButtonText("Generate");
generateButton.onClick = [this] {
    juce::String prompt = promptInput.getText();
    processor.generateInstrument(prompt);
};
addAndMakeVisible(generateButton);
```

### Timer for Updates
```cpp
class MyEditor : public juce::AudioProcessorEditor, 
                 private juce::Timer
{
    void timerCallback() override
    {
        statusLabel.setText(processor.getStatus(), juce::dontSendNotification);
        repaint();
    }
    
public:
    MyEditor()
    {
        startTimer(100);  // Update every 100ms
    }
};
```

## Threading

### Background Thread
```cpp
class GenerationThread : public juce::Thread
{
public:
    GenerationThread(Processor& p) : Thread("Generation"), processor(p) {}
    
    void run() override
    {
        // This runs on background thread
        auto result = callSlowOperation();
        
        // Update UI on message thread
        juce::MessageManager::callAsync([this, result]() {
            processor.loadResult(result);
        });
    }
    
private:
    Processor& processor;
};

// Usage
std::unique_ptr<GenerationThread> thread;
thread = std::make_unique<GenerationThread>(processor);
thread->startThread();
```

### Thread-Safe Communication
```cpp
class Processor
{
    std::atomic<bool> isGenerating { false };
    juce::String statusMessage;  // Only update from message thread!
    
    void updateStatus(const juce::String& msg)
    {
        juce::MessageManager::callAsync([this, msg]() {
            statusMessage = msg;  // Safe on message thread
        });
    }
};
```

## File I/O

### Load WAV File
```cpp
juce::AudioBuffer<float> loadWAV(const juce::String& path)
{
    juce::File file(path);
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    
    std::unique_ptr<juce::AudioFormatReader> reader(
        formatManager.createReaderFor(file)
    );
    
    if (reader != nullptr)
    {
        juce::AudioBuffer<float> buffer(
            reader->numChannels,
            reader->lengthInSamples
        );
        
        reader->read(&buffer, 0, reader->lengthInSamples, 0, true, true);
        return buffer;
    }
    
    return {};
}
```

### Save WAV File
```cpp
void saveWAV(const juce::AudioBuffer<float>& buffer, 
             const juce::String& path,
             double sampleRate = 44100.0)
{
    juce::File file(path);
    file.deleteFile();  // Overwrite if exists
    
    juce::WavAudioFormat wavFormat;
    std::unique_ptr<juce::AudioFormatWriter> writer(
        wavFormat.createWriterFor(
            new juce::FileOutputStream(file),
            sampleRate,
            buffer.getNumChannels(),
            16,  // Bit depth
            {},
            0
        )
    );
    
    if (writer != nullptr)
    {
        writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
    }
}
```

## Python AI Generation

### MusicGen Wrapper
```python
import torch
from audiocraft.models import MusicGen

class AudioGenerator:
    def __init__(self):
        self.model = MusicGen.get_pretrained('facebook/musicgen-small')
        self.model.set_generation_params(duration=3.0)
    
    def generate(self, prompt, duration=3.0):
        self.model.set_generation_params(duration=duration)
        
        with torch.no_grad():
            wav = self.model.generate([prompt])  # Shape: [1, channels, samples]
        
        # Save to file
        import torchaudio
        path = f'/tmp/generated_{time.time()}.wav'
        torchaudio.save(path, wav[0].cpu(), 44100)
        
        return path
```

## Common Patterns

### RAII File Handle
```cpp
void processTemporaryFile()
{
    juce::TemporaryFile tempFile(".wav");
    
    // Write to temp file
    saveWAV(buffer, tempFile.getFile().getFullPathName());
    
    // Process...
    
    // Automatic cleanup when tempFile goes out of scope
}
```

### Error Handling
```cpp
juce::Result loadSample(const juce::String& path)
{
    if (!juce::File(path).existsAsFile())
        return juce::Result::fail("File does not exist");
    
    auto buffer = loadWAV(path);
    
    if (buffer.getNumSamples() == 0)
        return juce::Result::fail("Failed to load audio");
    
    if (buffer.getNumSamples() < 1000)
        return juce::Result::fail("Sample too short");
    
    processSample(buffer);
    return juce::Result::ok();
}
```

### Logging
```cpp
// Debug log (only in debug builds)
DBG("Sample loaded: " + juce::String(buffer.getNumSamples()) + " samples");

// Always log
juce::Logger::writeToLog("Generation started for: " + prompt);

// Formatted
DBG(juce::String::formatted("Pitch detected: %.2f Hz (MIDI %d)", 
                            frequency, midiNote));
```

## Performance Tips

### Avoid Allocations in Audio Thread
```cpp
// BAD - allocates in audio thread
void processBlock(AudioBuffer<float>& buffer, MidiBuffer& midi)
{
    std::vector<int> notes;  // ❌ Allocation!
    for (auto msg : midi)
        notes.push_back(msg.getNoteNumber());
}

// GOOD - pre-allocated
class Processor
{
    std::vector<int> noteBuffer;  // Pre-allocated
    
    void prepareToPlay(double sr, int blockSize)
    {
        noteBuffer.reserve(128);  // Reserve once
    }
    
    void processBlock(AudioBuffer<float>& buffer, MidiBuffer& midi)
    {
        noteBuffer.clear();  // ✓ No allocation
        for (auto msg : midi)
            noteBuffer.push_back(msg.getNoteNumber());
    }
};
```

### Lock-Free Communication
```cpp
// Use atomic for simple flags
std::atomic<bool> shouldReload { false };

// Use juce::AbstractFifo for audio data
juce::AbstractFifo fifo(1024);
std::vector<float> buffer(1024);

// Writer thread
void pushData(float data)
{
    int start1, size1, start2, size2;
    fifo.prepareToWrite(1, start1, size1, start2, size2);
    if (size1 > 0)
    {
        buffer[start1] = data;
        fifo.finishedWrite(1);
    }
}
```

---

This reference covers 90% of common patterns you'll need when working on this plugin!
