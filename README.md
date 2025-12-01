# AI-Generated Instrument VST

A VST3/AU plugin that generates playable instruments from text prompts using AI. Enter a description like "deep bass synth" and the plugin will create a unique, MIDI-playable instrument.

## Features

- **Text-to-Instrument**: Generate sounds from natural language descriptions
- **Automatic Processing**: Auto-trim, normalize, pitch detection, and looping
- **MIDI Playable**: Works like any normal VSTi with full chromatic pitch shifting
- **Simple UI**: Minimal interface with prompt input and generate button
- **Local or Remote AI**: Supports local MusicGen model or remote APIs

## Quick Start

### 1. Install Dependencies

#### macOS
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install CMake (via Homebrew)
brew install cmake

# Clone JUCE framework
cd ~/Development
git clone https://github.com/juce-framework/JUCE.git
cd JUCE && git checkout 7.0.5
```

#### Windows
```powershell
# Install Visual Studio 2019 or later
# Install CMake from https://cmake.org/download/

# Clone JUCE framework
cd C:\Development
git clone https://github.com/juce-framework/JUCE.git
cd JUCE
git checkout 7.0.5
```

### 2. Build the Plugin

#### macOS
```bash
cd ~/CascadeProjects/aiGenVST
mkdir build && cd build

# Configure (adjust JUCE_PATH if needed)
cmake .. -DJUCE_PATH=~/Development/JUCE

# Build
cmake --build . --config Release

# Plugin will be in:
# VST3: ~/Library/Audio/Plug-Ins/VST3/AI Gen VST.vst3
# AU: ~/Library/Audio/Plug-Ins/Components/AI Gen VST.component
```

#### Windows
```powershell
cd C:\CascadeProjects\aiGenVST
mkdir build
cd build

# Configure
cmake .. -DJUCE_PATH=C:\Development\JUCE -G "Visual Studio 16 2019"

# Build
cmake --build . --config Release

# Plugin will be in:
# VST3: C:\Program Files\Common Files\VST3\AI Gen VST.vst3
```

### 3. Set Up Python Backend

```bash
cd python_backend

# Create virtual environment
python3 -m venv venv

# Activate (macOS/Linux)
source venv/bin/activate

# Activate (Windows)
# venv\Scripts\activate

# Install dependencies
pip install -r requirements.txt

# Start server
python server.py
```

The server will start on `http://localhost:5000`

### 4. Test the Plugin

1. Open your DAW (Ableton, FL Studio, Logic, etc.)
2. Load "AI Gen VST" as an instrument
3. Create a MIDI track
4. In the plugin window:
   - Make sure Python server is running
   - Enter a prompt (e.g., "bell sound")
   - Click "Generate Instrument"
   - Wait 5-15 seconds for generation
5. Play MIDI notes to hear your generated instrument!

## Usage

### Example Prompts

- `"deep bass synth"`
- `"bell sound"`
- `"piano note"`
- `"electric guitar pluck"`
- `"808 kick drum"`
- `"ambient pad"`
- `"brass stab"`

### Tips

- Be specific but concise (3-6 words work best)
- Focus on timbre, not melody
- For percussive sounds, use short durations
- For pads/sustained sounds, use longer durations
- Experiment! The AI is creative

## Architecture

```
┌─────────────┐      MIDI      ┌──────────────┐
│     DAW     │ ────────────► │  VST Plugin  │
│             │ ◄──────────── │   (C++/JUCE) │
└─────────────┘     Audio      └──────┬───────┘
                                       │
                                       │ HTTP
                                       │
                                ┌──────▼───────┐
                                │   Python     │
                                │   Backend    │
                                │  (MusicGen)  │
                                └──────────────┘
```

### Components

1. **Plugin (C++)**:
   - JUCE-based VST3/AU plugin
   - Sampler engine with pitch shifting
   - ADSR envelope, looping
   - HTTP client for AI requests

2. **Python Backend**:
   - Flask server on port 5000
   - Meta MusicGen model (300M params)
   - Audio processing pipeline

3. **Communication**:
   - JSON over HTTP
   - Plugin sends prompt → Python generates → returns WAV path

## Configuration

### Change AI Model

Edit `python_backend/generator.py`:

```python
# Faster, lower quality
gen = AudioGenerator('facebook/musicgen-small')

# Slower, higher quality
gen = AudioGenerator('facebook/musicgen-medium')
```

### Change Server Port

Edit `python_backend/server.py`:
```python
app.run(host='0.0.0.0', port=5000)  # Change port here
```

Edit `Source/AIGenerator.h`:
```cpp
juce::String serverURL = "http://localhost:5000";  // Update URL
```

### Remote API (Optional)

Instead of local Python server, use Replicate API:

1. Sign up at https://replicate.com
2. Get API token
3. Modify `AIGenerator.cpp` to call Replicate endpoint
4. Example: https://replicate.com/meta/musicgen

## Performance

### Generation Time
- **CPU (MusicGen-small)**: 8-15 seconds for 3s audio
- **GPU (MusicGen-small)**: 2-4 seconds for 3s audio
- **Remote API**: 10-30 seconds (depends on queue)

### System Requirements
- **RAM**: 4GB minimum, 8GB recommended
- **Storage**: ~2GB for model
- **CPU**: Multi-core recommended
- **GPU**: Optional, CUDA-compatible for faster generation

## Troubleshooting

### "Failed to connect to server"
- Ensure Python backend is running (`python server.py`)
- Check firewall settings
- Verify port 5000 is not in use

### "No sound output"
- Check MIDI is routed to the plugin
- Verify DAW's audio settings
- Try regenerating the instrument
- Check plugin state in DAW

### "Out of memory"
- Use `musicgen-small` instead of `medium`
- Reduce generation duration
- Close other applications

### "Generation takes too long"
- First generation loads model (~10-20s)
- Subsequent generations are faster
- Consider using GPU
- Try remote API for consistency

## Development

### Project Structure
```
aiGenVST/
├── Source/
│   ├── PluginProcessor.h/cpp    # Main audio processor
│   ├── PluginEditor.h/cpp       # UI components
│   ├── SamplerEngine.h/cpp      # Sampler with voices
│   ├── PitchDetector.h/cpp      # Autocorrelation pitch detect
│   └── AIGenerator.h/cpp        # HTTP client
├── python_backend/
│   ├── server.py                # Flask server
│   ├── generator.py             # MusicGen wrapper
│   └── requirements.txt
├── CMakeLists.txt
└── README.md
```

### Adding Features

See [STRETCH_GOALS.md](STRETCH_GOALS.md) for expansion ideas:
- Multi-sampling across MIDI range
- Wavetable extraction
- Effects chain (filter, reverb)
- Preset saving/loading
- Real-time parameter control

### Debugging

**C++ Plugin:**
```bash
# Build in debug mode
cmake --build . --config Debug

# Run standalone version
./build/AIGenVST_artefacts/Debug/Standalone/AI\ Gen\ VST
```

**Python Backend:**
```bash
# Test generation directly
python generator.py

# Test server
curl -X POST http://localhost:5000/generate \
  -H "Content-Type: application/json" \
  -d '{"prompt": "test", "duration": 2.0}'
```

## License

This project is provided as-is for educational purposes.

- JUCE: GPL/Commercial (see JUCE licensing)
- MusicGen: CC-BY-NC-4.0 (non-commercial)

For commercial use, you must:
1. Obtain JUCE commercial license
2. Use a commercially-licensed AI model or API

## Credits

- **JUCE Framework**: Audio plugin framework
- **Meta AudioCraft**: MusicGen model
- **PyTorch**: ML framework

## Support

For issues, questions, or contributions:
1. Check troubleshooting section
2. Review [ARCHITECTURE.md](ARCHITECTURE.md)
3. Open an issue on GitHub

---

**Made with 🎵 and AI**
# aiGenVST
