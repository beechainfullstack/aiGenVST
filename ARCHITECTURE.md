# AI-Generated Instrument VST - Architecture

## High-Level System Design

```
┌─────────────────────────────────────────────────────────────┐
│                    DAW (Host Application)                    │
└───────────────────┬─────────────────────────────────────────┘
                    │ MIDI In / Audio Out
                    │
┌───────────────────▼─────────────────────────────────────────┐
│                   VST Plugin (JUCE)                          │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  UI Layer (PluginEditor)                             │   │
│  │  - Text prompt input                                 │   │
│  │  - Generate button                                   │   │
│  │  - Loading spinner                                   │   │
│  │  - Sample preview waveform                           │   │
│  └─────────┬────────────────────────────────────────────┘   │
│            │                                                 │
│  ┌─────────▼────────────────────────────────────────────┐   │
│  │  Audio Processor (PluginProcessor)                   │   │
│  │  - MIDI input handling                               │   │
│  │  - Audio output generation                           │   │
│  │  - Parameter state management                        │   │
│  └─────────┬────────────────────────────────────────────┘   │
│            │                                                 │
│  ┌─────────▼────────────────────────────────────────────┐   │
│  │  Sampler Engine                                      │   │
│  │  - Voice management (polyphony)                      │   │
│  │  - MIDI note → pitch mapping                         │   │
│  │  - ADSR envelope                                     │   │
│  │  - Sample playback with pitch shifting               │   │
│  └─────────┬────────────────────────────────────────────┘   │
│            │                                                 │
│  ┌─────────▼────────────────────────────────────────────┐   │
│  │  Sample Manager                                      │   │
│  │  - Load WAV from disk                                │   │
│  │  - Auto-normalize                                    │   │
│  │  - Auto-trim silence                                 │   │
│  │  - Loop point detection                              │   │
│  │  - Pitch detection (root note)                       │   │
│  └─────────┬────────────────────────────────────────────┘   │
│            │                                                 │
│  ┌─────────▼────────────────────────────────────────────┐   │
│  │  AI Inference Manager                                │   │
│  │  - Text → Audio generation                           │   │
│  │  - Background thread handling                        │   │
│  │  - Progress callbacks                                │   │
│  └─────────┬────────────────────────────────────────────┘   │
└────────────┼─────────────────────────────────────────────────┘
             │
             │ HTTP/IPC
             │
┌────────────▼─────────────────────────────────────────────────┐
│         AI Generation Backend (Python)                       │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  Option A: Local Model                               │   │
│  │  - AudioCraft MusicGen Small                         │   │
│  │  - Stable Audio Open                                 │   │
│  │  - Riffusion                                         │   │
│  └──────────────────────────────────────────────────────┘   │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  Option B: Remote API                                │   │
│  │  - Replicate API                                     │   │
│  │  - Stability AI API                                  │   │
│  │  - OpenAI API                                        │   │
│  └──────────────────────────────────────────────────────┘   │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  Flask/FastAPI Server                                │   │
│  │  POST /generate                                      │   │
│  │  {"prompt": "...", "duration": 5.0}                  │   │
│  │  → returns WAV file path                             │   │
│  └──────────────────────────────────────────────────────┘   │
└──────────────────────────────────────────────────────────────┘
```

## Tech Stack

### Core Plugin (C++)
- **JUCE Framework 7.x**: Cross-platform audio plugin framework
- **C++17**: Modern C++ with std::optional, std::variant
- **VST3 SDK**: Embedded in JUCE
- **CMake 3.15+**: Build system

### Audio Processing
- **JUCE AudioSampleBuffer**: Audio buffer management
- **JUCE Synthesiser**: Built-in sampler/voice architecture
- **libsamplerate or JUCE resampling**: Pitch shifting
- **Essentia or custom FFT**: Pitch detection (optional)

### AI Backend (Python)
- **Python 3.9+**
- **FastAPI or Flask**: HTTP server
- **PyTorch**: Model inference
- **audiocraft**: MusicGen model
- **scipy/soundfile**: Audio I/O

### Communication
- **cURL or JUCE URL**: HTTP client in C++
- **JSON**: Request/response format
- **Localhost HTTP**: Simple IPC

## Data Flow

1. **User Input** → UI Editor receives text prompt
2. **Generate Click** → Processor spawns background thread
3. **HTTP Request** → C++ calls Python server at `http://localhost:5000/generate`
4. **AI Inference** → Python loads model, generates audio (3-10s)
5. **WAV File** → Python saves to temp file, returns path
6. **Sample Load** → C++ loads WAV into buffer
7. **Audio Processing**:
   - Detect silence, trim start/end
   - Normalize to -0.5 dBFS
   - Detect pitch using autocorrelation or YIN algorithm
   - Set root note (e.g., C3 = MIDI 60)
   - Find loop points (zero-crossing analysis)
8. **Sampler Ready** → User plays MIDI notes, sampler transposes

## Component Details

### 1. Sampler Engine
- **Polyphony**: 16 voices
- **Pitch shifting**: Granular resampling (JUCE built-in)
- **Envelope**: Simple ADSR
- **Root note**: Detected or default to C3
- **Loop mode**: Forward loop with crossfade

### 2. Sample Processing Pipeline
```
Raw WAV → Trim Silence → Normalize → Pitch Detect → Loop Detect → Ready
```

### 3. AI Model Options (Simplest First)

**Recommended for MVP: MusicGen Small**
- **Model**: `facebook/musicgen-small` (300M params)
- **Speed**: ~8-12s generation on CPU, ~2-3s on GPU
- **Quality**: Good for simple instruments
- **Memory**: ~1.5GB RAM

**Alternative: Riffusion**
- **Model**: `riffusion/riffusion-model-v1`
- **Speed**: ~5-7s on CPU
- **Quality**: Works well for synthesizer sounds
- **Memory**: ~3GB RAM

**Remote Fallback: Replicate API**
- **Endpoint**: `https://api.replicate.com/v1/predictions`
- **Model**: `meta/musicgen-small`
- **Cost**: $0.0023/second of audio
- **Latency**: ~10-30s depending on queue

## File Structure (MVP)

```
aiGenVST/
├── CMakeLists.txt                 # Build config
├── Source/
│   ├── PluginProcessor.h/cpp      # Audio processing + MIDI
│   ├── PluginEditor.h/cpp         # UI components
│   ├── SamplerEngine.h/cpp        # Voice + playback
│   ├── SampleManager.h/cpp        # WAV processing
│   ├── AIGenerator.h/cpp          # HTTP client for AI
│   └── PitchDetector.h/cpp        # Autocorrelation pitch detection
├── python_backend/
│   ├── server.py                  # FastAPI server
│   ├── generator.py               # Model wrapper
│   └── requirements.txt           # Dependencies
├── README.md                      # Build + usage
└── ARCHITECTURE.md                # This file
```

## Build Requirements

### macOS
- Xcode 13+
- CMake 3.15+
- JUCE 7.0.5+

### Windows
- Visual Studio 2019+
- CMake 3.15+
- JUCE 7.0.5+

### Linux (Optional)
- GCC 9+ or Clang 10+
- CMake 3.15+
- JUCE dependencies (see JUCE docs)

## Simplifications for MVP

1. **No wavetable**: Just sampler with pitch shift
2. **No real-time generation**: Blocking UI is OK
3. **Single sample**: No multi-sampling across MIDI ranges
4. **No MIDI velocity layers**: Single layer
5. **No effects**: No reverb/delay/filter
6. **No preset saving**: Manual generation each time

## Next Steps

1. Set up JUCE project
2. Implement basic sampler with sine wave
3. Add UI for prompt input
4. Create Python server with dummy response
5. Wire up HTTP client
6. Integrate real AI model
7. Add sample processing (trim, normalize, pitch detect)
8. Test in DAW
