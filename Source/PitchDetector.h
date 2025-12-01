#pragma once

#include <JuceHeader.h>

//==============================================================================
// Simple autocorrelation-based pitch detector
class PitchDetector
{
public:
    PitchDetector();
    
    // Detects fundamental frequency in Hz
    // Returns 0.0 if no pitch detected
    float detectPitch(const juce::AudioBuffer<float>& buffer, double sampleRate);

private:
    float autocorrelate(const float* data, int length, int lag);
    
    static constexpr int minPeriod = 20;    // ~2200 Hz max
    static constexpr int maxPeriod = 2000;  // ~22 Hz min
};
