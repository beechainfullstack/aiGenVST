# Implementation Plan - AI VST Plugin

## Phase 1: Project Setup (30 minutes)

### Step 1.1: Install JUCE
```bash
cd ~/Development
git clone https://github.com/juce-framework/JUCE.git
cd JUCE
git checkout 7.0.5
```

### Step 1.2: Create CMake Project
```bash
cd ~/CascadeProjects/aiGenVST
# Use provided CMakeLists.txt
```

### Step 1.3: Verify Build
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## Phase 2: Core Sampler Engine (2-3 hours)

### Step 2.1: Implement SamplerVoice
- Extend `juce::SynthesiserVoice`
- Add pitch shifting using playback rate
- Implement ADSR envelope
- Add loop point support

### Step 2.2: Implement SamplerSound
- Extend `juce::SynthesiserSound`
- Store audio buffer
- Store root note MIDI number
- Store loop start/end points

### Step 2.3: Test with Sine Wave
- Generate test sine wave in processor
- Verify MIDI input triggers voices
- Verify pitch shifting works

**Expected Output**: Can load a WAV and play it chromatically via MIDI

## Phase 3: Sample Processing (2-3 hours)

### Step 3.1: Audio Loading
- Load WAV using JUCE AudioFormatReader
- Convert to mono if stereo
- Resample to 44.1kHz if needed

### Step 3.2: Auto-Trim Silence
- Detect silence threshold (-60 dB)
- Find first/last non-silent sample
- Trim buffer

### Step 3.3: Normalization
- Find peak amplitude
- Apply gain to reach -0.5 dBFS

### Step 3.4: Pitch Detection
- Implement autocorrelation (YIN algorithm)
- Detect fundamental frequency
- Convert to MIDI note number
- Default to C3 (MIDI 60) if detection fails

### Step 3.5: Loop Point Detection
- Find zero-crossings near sample end
- Set loop points for seamless playback

**Expected Output**: Raw WAV → Processed, loopable sample

## Phase 4: UI Components (2 hours)

### Step 4.1: Create Editor Layout
- Text input field (prompt)
- Generate button
- Loading indicator
- Waveform display (optional for MVP)

### Step 4.2: Wire UI to Processor
- Button click → triggers generation
- Callback updates UI when done
- Display error messages

**Expected Output**: Functional UI in plugin window

## Phase 5: Python AI Backend (3-4 hours)

### Step 5.1: Set Up Python Environment
```bash
cd python_backend
python3 -m venv venv
source venv/bin/activate  # Windows: venv\Scripts\activate
pip install -r requirements.txt
```

### Step 5.2: Create FastAPI Server
- POST `/generate` endpoint
- Accept: `{"prompt": "string", "duration": float}`
- Return: `{"wav_path": "string"}`

### Step 5.3: Integrate MusicGen
- Load `musicgen-small` model
- Generate audio from prompt
- Save to temp WAV file
- Return file path

### Step 5.4: Test Server
```bash
python server.py
curl -X POST http://localhost:5000/generate \
  -H "Content-Type: application/json" \
  -d '{"prompt": "bell sound", "duration": 3.0}'
```

**Expected Output**: Python server generates WAV files

## Phase 6: C++ HTTP Client (1-2 hours)

### Step 6.1: Implement AIGenerator
- Use JUCE URL or libcurl
- POST JSON to Python server
- Parse response
- Download WAV file

### Step 6.2: Background Thread
- Spawn thread for HTTP request
- Update UI with progress
- Callback when complete

### Step 6.3: Error Handling
- Timeout (30s)
- Connection errors
- Invalid responses

**Expected Output**: C++ plugin can call Python server

## Phase 7: Integration (1-2 hours)

### Step 7.1: Connect All Components
```
UI Generate Button
  → AIGenerator::generateAsync(prompt)
    → HTTP POST to Python
      → Wait for response
        → SampleManager::loadAndProcess(wavPath)
          → SamplerEngine::loadSample(processedBuffer)
            → UI update: Ready to play
```

### Step 7.2: Test End-to-End
1. Load plugin in DAW
2. Enter prompt: "piano note"
3. Click generate
4. Wait for loading
5. Play MIDI notes
6. Verify sound output

**Expected Output**: Full workflow works

## Phase 8: Polish & Testing (2 hours)

### Step 8.1: Add UI Feedback
- Show generation progress
- Display loaded sample info
- Error messages

### Step 8.2: Test Edge Cases
- Empty prompt
- Server not running
- Invalid WAV files
- Very short/long samples

### Step 8.3: Optimize Performance
- Ensure audio thread is real-time safe
- Move processing to background threads

**Expected Output**: Stable, usable plugin

## Phase 9: Documentation (1 hour)

### Step 9.1: Write README
- Installation instructions
- Build steps
- Usage guide
- Troubleshooting

### Step 9.2: Document API
- Comment code
- Explain key algorithms

## Total Estimated Time: 12-18 hours

## Testing Checklist

- [ ] Plugin loads in DAW (Ableton, FL Studio, etc.)
- [ ] MIDI input triggers sound
- [ ] Pitch shifting works correctly
- [ ] Generate button creates new instrument
- [ ] Loading indicator appears during generation
- [ ] Multiple generations work (no memory leaks)
- [ ] Plugin saves/loads state
- [ ] Works on macOS and Windows
- [ ] No crashes under normal use
- [ ] Audio thread is glitch-free

## Common Issues & Solutions

### Python server not starting
- Check port 5000 is available
- Verify dependencies installed
- Check Python version (3.9+)

### No sound output
- Check MIDI is being received
- Verify sample loaded successfully
- Check DAW routing

### Crashes on generation
- Add exception handling
- Increase timeout
- Check thread safety

### Poor audio quality
- Verify sample rate conversion
- Check normalization isn't clipping
- Test pitch detection accuracy
