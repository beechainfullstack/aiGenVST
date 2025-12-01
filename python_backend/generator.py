"""
Audio Generator using Meta's MusicGen model
"""

import torch
import torchaudio
from audiocraft.models import MusicGen
import tempfile
import os
import logging

logger = logging.getLogger(__name__)

class AudioGenerator:
    """
    Wrapper for MusicGen audio generation
    """
    
    def __init__(self, model_name='facebook/musicgen-small'):
        """
        Initialize the audio generator
        
        Args:
            model_name: HuggingFace model name
                - 'facebook/musicgen-small' (300M params, fastest)
                - 'facebook/musicgen-medium' (1.5B params, better quality)
        """
        logger.info(f"Loading model: {model_name}")
        
        # Load model
        self.model = MusicGen.get_pretrained(model_name)
        
        # Set generation parameters
        self.sample_rate = 32000  # MusicGen default
        
        # Use GPU if available
        self.device = 'cuda' if torch.cuda.is_available() else 'cpu'
        logger.info(f"Using device: {self.device}")
        
        if self.device == 'cuda':
            self.model = self.model.to('cuda')
    
    def generate(self, prompt, duration=3.0):
        """
        Generate audio from text prompt
        
        Args:
            prompt: Text description of desired audio
            duration: Length of audio in seconds
        
        Returns:
            Path to generated WAV file
        """
        logger.info(f"Generating: '{prompt}' for {duration}s")
        
        # Set duration
        self.model.set_generation_params(duration=duration)
        
        # Generate audio
        with torch.no_grad():
            wav = self.model.generate([prompt])  # Returns [1, channels, samples]
        
        # Convert to CPU and numpy
        wav = wav.cpu()
        
        # Save to temporary file
        temp_file = tempfile.NamedTemporaryFile(delete=False, suffix='.wav', dir='/tmp')
        temp_path = temp_file.name
        temp_file.close()
        
        # Save as WAV (convert to 44.1kHz for compatibility)
        logger.info(f"Saving to: {temp_path}")
        
        # Resample to 44.1kHz if needed
        if self.sample_rate != 44100:
            resampler = torchaudio.transforms.Resample(
                orig_freq=self.sample_rate,
                new_freq=44100
            )
            wav = resampler(wav)
        
        # Save
        torchaudio.save(temp_path, wav[0], 44100)
        
        logger.info("Generation complete")
        return temp_path

# Test code
if __name__ == '__main__':
    print("Testing AudioGenerator...")
    
    gen = AudioGenerator()
    
    test_prompts = [
        "deep bass synth",
        "bell sound",
        "piano note",
    ]
    
    for prompt in test_prompts:
        print(f"\nGenerating: {prompt}")
        path = gen.generate(prompt, duration=2.0)
        print(f"Saved to: {path}")
        
        # Check file size
        size_mb = os.path.getsize(path) / (1024 * 1024)
        print(f"File size: {size_mb:.2f} MB")
