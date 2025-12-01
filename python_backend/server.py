#!/usr/bin/env python3
"""
AI Audio Generation Server for VST Plugin
Uses MusicGen model to generate audio from text prompts
"""

from flask import Flask, request, jsonify
import os
import tempfile
import logging
from generator import AudioGenerator

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

app = Flask(__name__)

# Initialize generator (lazy loading)
generator = None

def get_generator():
    """Lazy load the audio generator"""
    global generator
    if generator is None:
        logger.info("Loading AI model...")
        generator = AudioGenerator()
        logger.info("Model loaded successfully")
    return generator

@app.route('/health', methods=['GET'])
def health_check():
    """Health check endpoint"""
    return jsonify({"status": "ok", "model_loaded": generator is not None})

@app.route('/generate', methods=['POST'])
def generate():
    """
    Generate audio from text prompt
    
    Request JSON:
    {
        "prompt": "deep bass synth",
        "duration": 3.0
    }
    
    Response JSON:
    {
        "wav_path": "/tmp/generated_xyz.wav"
    }
    """
    try:
        # Parse request
        data = request.get_json()
        
        if not data:
            return jsonify({"error": "No JSON data provided"}), 400
        
        prompt = data.get('prompt', '')
        duration = data.get('duration', 3.0)
        
        if not prompt:
            return jsonify({"error": "Prompt cannot be empty"}), 400
        
        logger.info(f"Generating audio for prompt: '{prompt}' ({duration}s)")
        
        # Generate audio
        gen = get_generator()
        wav_path = gen.generate(prompt, duration)
        
        logger.info(f"Audio generated: {wav_path}")
        
        return jsonify({
            "wav_path": wav_path,
            "prompt": prompt,
            "duration": duration
        })
    
    except Exception as e:
        logger.error(f"Generation error: {str(e)}", exc_info=True)
        return jsonify({"error": str(e)}), 500

@app.route('/test', methods=['GET'])
def test():
    """Test endpoint that generates a simple sine wave"""
    import numpy as np
    import soundfile as sf
    
    # Generate simple sine wave
    duration = 1.0
    sample_rate = 44100
    frequency = 440.0  # A4
    
    t = np.linspace(0, duration, int(sample_rate * duration))
    audio = 0.5 * np.sin(2 * np.pi * frequency * t)
    
    # Save to temp file
    temp_file = tempfile.NamedTemporaryFile(delete=False, suffix='.wav')
    sf.write(temp_file.name, audio, sample_rate)
    
    return jsonify({"wav_path": temp_file.name})

if __name__ == '__main__':
    print("=" * 60)
    print("AI Audio Generation Server")
    print("=" * 60)
    print("Starting server on http://localhost:5000")
    print("Endpoints:")
    print("  POST /generate - Generate audio from prompt")
    print("  GET  /health   - Health check")
    print("  GET  /test     - Generate test sine wave")
    print("=" * 60)
    
    # Run server
    app.run(host='0.0.0.0', port=5000, debug=False, threaded=True)
