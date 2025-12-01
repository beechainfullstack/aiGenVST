# Quick Start Guide

Get the AI VST plugin running in 15 minutes.

## Option A: Full Setup (Local AI Model)

### 1. Install JUCE (5 minutes)
```bash
cd ~/Development
git clone https://github.com/juce-framework/JUCE.git
cd JUCE && git checkout 7.0.5
```

### 2. Build Plugin (5 minutes)
```bash
cd ~/CascadeProjects/aiGenVST
mkdir build && cd build
cmake .. -DJUCE_PATH=~/Development/JUCE
cmake --build . --config Release
```

### 3. Start Python Backend (5 minutes)
```bash
cd ../python_backend
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
python server.py  # First run downloads ~2GB model
```

### 4. Test in DAW
1. Open Ableton/Logic/FL Studio
2. Load "AI Gen VST" instrument
3. Enter prompt: "bell sound"
4. Click Generate
5. Play MIDI notes!

---

## Option B: Test Mode (No AI, 2 minutes)

Use test endpoint for development without downloading AI models.

### Modify AIGenerator.cpp
```cpp
GenerationResult AIGenerator::generate(const juce::String& prompt, float duration)
{
    // Use test endpoint instead
    juce::URL url(serverURL + "/test");  // Changed from /generate
    
    std::unique_ptr<juce::InputStream> stream(url.createInputStream());
    // ... rest of code
}
```

### Start Simple Server
```bash
cd python_backend
python3 server.py  # No dependencies needed for /test endpoint
```

This generates a simple sine wave instead of AI audio - perfect for testing the plugin infrastructure!

---

## Option C: Remote API (No Local Setup)

### 1. Get Replicate API Key
Sign up at https://replicate.com and get API token

### 2. Modify Python Backend
```python
# In python_backend/server.py
import requests

@app.route('/generate', methods=['POST'])
def generate():
    data = request.get_json()
    
    # Call Replicate API
    response = requests.post(
        "https://api.replicate.com/v1/predictions",
        headers={"Authorization": f"Token {REPLICATE_API_TOKEN}"},
        json={
            "version": "MODEL_VERSION_HERE",
            "input": {
                "prompt": data['prompt'],
                "duration": data['duration']
            }
        }
    )
    
    # Wait for result and download WAV
    # ... implementation details
```

This avoids local model but requires internet and API costs (~$0.01/generation).

---

## Troubleshooting

### "JUCE not found"
```bash
# Set explicit path
cmake .. -DJUCE_PATH=/full/path/to/JUCE
```

### "Python dependencies fail"
```bash
# Update pip first
pip install --upgrade pip
pip install --upgrade setuptools wheel

# Then try again
pip install -r requirements.txt
```

### "Out of memory" (when loading model)
```bash
# Use smaller model in generator.py
model = MusicGen.get_pretrained('small')  # instead of 'medium'
```

### "Port 5000 already in use"
```bash
# Kill existing process
lsof -ti:5000 | xargs kill -9

# Or change port in server.py and AIGenerator.h
```

---

## Next Steps

1. **Read ARCHITECTURE.md** - Understand the system
2. **Try example prompts** - See what works
3. **Customize** - Modify code for your needs
4. **Add features** - Check STRETCH_GOALS.md

---

## Development Workflow

```bash
# Terminal 1: Run Python backend
cd python_backend
source venv/bin/activate
python server.py

# Terminal 2: Rebuild plugin when making changes
cd build
cmake --build . --config Release

# Terminal 3: Watch logs
tail -f /var/log/system.log  # macOS
```

---

## File Locations

### macOS
- **VST3**: `~/Library/Audio/Plug-Ins/VST3/AI Gen VST.vst3`
- **AU**: `~/Library/Audio/Plug-Ins/Components/AI Gen VST.component`
- **Standalone**: `build/AIGenVST_artefacts/Release/Standalone/`

### Windows
- **VST3**: `C:\Program Files\Common Files\VST3\AI Gen VST.vst3`
- **Standalone**: `build\AIGenVST_artefacts\Release\Standalone\`

---

## First Generation Timeline

1. **Start plugin** - Instant
2. **Enter prompt** - 5 seconds
3. **Click generate** - Instant
4. **First HTTP request** - 2 seconds
5. **Load AI model** (first time) - 15-20 seconds
6. **Generate audio** - 8-12 seconds
7. **Process sample** - 1 second
8. **Ready to play** - Instant

**Total first generation**: ~30-40 seconds

**Subsequent generations**: ~10-15 seconds (model already loaded)

---

**You're ready! Start generating instruments!** 🎵
