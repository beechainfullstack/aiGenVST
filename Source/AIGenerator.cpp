#include "AIGenerator.h"

AIGenerator::AIGenerator()
{
}

GenerationResult AIGenerator::generate(const juce::String& prompt, float duration)
{
    return sendHTTPRequest(prompt, duration);
}

GenerationResult AIGenerator::sendHTTPRequest(const juce::String& prompt, float duration)
{
    GenerationResult result;
    
    try
    {
        // Build JSON request
        juce::DynamicObject::Ptr jsonObject = new juce::DynamicObject();
        jsonObject->setProperty("prompt", prompt);
        jsonObject->setProperty("duration", duration);
        
        juce::var jsonVar(jsonObject.get());
        juce::String jsonString = juce::JSON::toString(jsonVar);
        
        // Create URL
        juce::URL url(serverURL + "/generate");
        
        // Set up HTTP request
        juce::URL::InputStreamOptions options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
            .withConnectionTimeoutMs(timeoutSeconds * 1000)
            .withExtraHeaders("Content-Type: application/json");
        
        // Send POST request
        std::unique_ptr<juce::InputStream> stream(url.createInputStream(
            options.withHttpRequestCmd("POST").withPostData(jsonString)
        ));
        
        if (stream == nullptr)
        {
            result.errorMessage = "Failed to connect to server at " + serverURL;
            return result;
        }
        
        // Read response
        juce::String response = stream->readEntireStreamAsString();
        
        // Parse JSON response
        juce::var parsedJson;
        juce::Result parseResult = juce::JSON::parse(response, parsedJson);
        
        if (!parseResult.wasOk())
        {
            result.errorMessage = "Failed to parse server response: " + parseResult.getErrorMessage();
            return result;
        }
        
        // Extract WAV file path
        if (parsedJson.hasProperty("wav_path"))
        {
            result.wavFilePath = parsedJson["wav_path"].toString();
            result.success = true;
        }
        else if (parsedJson.hasProperty("error"))
        {
            result.errorMessage = parsedJson["error"].toString();
        }
        else
        {
            result.errorMessage = "Invalid response format from server";
        }
    }
    catch (const std::exception& e)
    {
        result.errorMessage = "Exception: " + juce::String(e.what());
    }
    
    return result;
}
