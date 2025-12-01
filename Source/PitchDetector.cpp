#include "PitchDetector.h"

PitchDetector::PitchDetector()
{
}

float PitchDetector::detectPitch(const juce::AudioBuffer<float>& buffer, double sampleRate)
{
    if (buffer.getNumSamples() < maxPeriod * 2)
        return 0.0f;
    
    const float* data = buffer.getReadPointer(0);
    int length = juce::jmin(buffer.getNumSamples(), 8192); // Analyze first ~185ms
    
    // Find the lag with maximum autocorrelation (excluding lag 0)
    float maxCorrelation = -1.0f;
    int bestLag = 0;
    
    for (int lag = minPeriod; lag < maxPeriod && lag < length / 2; ++lag)
    {
        float correlation = autocorrelate(data, length, lag);
        
        if (correlation > maxCorrelation)
        {
            maxCorrelation = correlation;
            bestLag = lag;
        }
    }
    
    // Require minimum correlation for confidence
    if (maxCorrelation < 0.3f || bestLag == 0)
        return 0.0f;
    
    // Convert lag to frequency
    float frequency = (float)sampleRate / (float)bestLag;
    
    return frequency;
}

float PitchDetector::autocorrelate(const float* data, int length, int lag)
{
    float sum = 0.0f;
    float norm1 = 0.0f;
    float norm2 = 0.0f;
    
    for (int i = 0; i < length - lag; ++i)
    {
        sum += data[i] * data[i + lag];
        norm1 += data[i] * data[i];
        norm2 += data[i + lag] * data[i + lag];
    }
    
    // Normalized autocorrelation
    float normalization = std::sqrt(norm1 * norm2);
    if (normalization > 0.0f)
        return sum / normalization;
    
    return 0.0f;
}
