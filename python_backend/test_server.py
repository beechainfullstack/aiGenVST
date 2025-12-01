#!/usr/bin/env python3
"""
Lightweight test server that generates simple waveforms
Use this for testing the plugin without downloading AI models
"""

from flask import Flask, request, jsonify
import numpy as np
import soundfile as sf
import tempfile
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

app = Flask(__name__)

def generate_test_audio(prompt, duration=3.0):
    """
    Generate test audio based on prompt keywords
    No AI - just simple synthesis for testing
    """
    sample_rate = 44100
    t = np.linspace(0, duration, int(sample_rate * duration))
    
    # Different waveforms based on prompt
    if 'sine' in prompt.lower() or 'bell' in prompt.lower():
        # Sine wave with decay
        frequency = 440.0
        audio = np.sin(2 * np.pi * frequency * t)
        envelope = np.exp(-t * 2.0)  # Decay
        audio *= envelope
    
    elif 'saw' in prompt.lower() or 'bass' in prompt.lower():
        # Sawtooth wave
        frequency = 110.0
        audio = 2 * (t * frequency - np.floor(t * frequency + 0.5))
        envelope = np.exp(-t * 1.0)
        audio *= envelope * 0.5
    
    elif 'square' in prompt.lower() or 'lead' in prompt.lower():
        # Square wave
        frequency = 220.0
        audio = np.sign(np.sin(2 * np.pi * frequency * t))
        envelope = np.exp(-t * 1.5)
        audio *= envelope * 0.3
    
    else:
        # Default: Sine wave
        frequency = 440.0
        audio = np.sin(2 * np.pi * frequency * t)
        envelope = np.exp(-t * 2.0)
        audio *= envelope
    
    # Normalize
    audio = audio / np.max(np.abs(audio)) * 0.8
    
    return audio.astype(np.float32)

@app.route('/health', methods=['GET'])
def health():
    return jsonify({"status": "ok", "mode": "test"})

@app.route('/generate', methods=['POST'])
def generate():
    try:
        data = request.get_json()
        prompt = data.get('prompt', 'sine')
        duration = data.get('duration', 3.0)
        
        logger.info(f"Test generation: '{prompt}' ({duration}s)")
        
        # Generate test audio
        audio = generate_test_audio(prompt, duration)
        
        # Save to temp file
        temp_file = tempfile.NamedTemporaryFile(delete=False, suffix='.wav', dir='/tmp')
        sf.write(temp_file.name, audio, 44100)
        temp_file.close()
        
        logger.info(f"Test audio saved: {temp_file.name}")
        
        return jsonify({
            "wav_path": temp_file.name,
            "prompt": prompt,
            "duration": duration,
            "mode": "test"
        })
    
    except Exception as e:
        logger.error(f"Error: {e}")
        return jsonify({"error": str(e)}), 500

if __name__ == '__main__':
    print("=" * 60)
    print("TEST SERVER - No AI Models Required")
    print("=" * 60)
    print("This server generates simple test waveforms")
    print("Perfect for testing the plugin without downloading models")
    print()
    print("Starting on http://localhost:5000")
    print()
    print("Test prompts:")
    print("  - 'sine' or 'bell' → Sine wave")
    print("  - 'saw' or 'bass' → Sawtooth wave")
    print("  - 'square' or 'lead' → Square wave")
    print("=" * 60)
    
    app.run(host='0.0.0.0', port=5000, debug=False)
