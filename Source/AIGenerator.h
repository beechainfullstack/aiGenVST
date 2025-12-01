#pragma once

#include <JuceHeader.h>

//==============================================================================
struct GenerationResult
{
    bool success = false;
    juce::String wavFilePath;
    juce::String errorMessage;
};

//==============================================================================
// AI Generator client - communicates with Python backend
class AIGenerator
{
public:
    AIGenerator();
    
    // Synchronous generation (blocks until complete)
    GenerationResult generate(const juce::String& prompt, float duration = 3.0f);
    
    // Configuration
    void setServerURL(const juce::String& url) { serverURL = url; }
    void setTimeout(int seconds) { timeoutSeconds = seconds; }

private:
    juce::String serverURL = "http://localhost:5000";
    int timeoutSeconds = 60;
    
    GenerationResult sendHTTPRequest(const juce::String& prompt, float duration);
};
