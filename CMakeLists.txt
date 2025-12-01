cmake_minimum_required(VERSION 3.15)
project(AIGenVST VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE
# You need to set JUCE_PATH to your JUCE installation
if(NOT DEFINED JUCE_PATH)
    set(JUCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE" CACHE PATH "Path to JUCE framework")
endif()

add_subdirectory(${JUCE_PATH} JUCE)

# Define plugin target
juce_add_plugin(AIGenVST
    COMPANY_NAME "YourCompany"
    PLUGIN_MANUFACTURER_CODE Yuco
    PLUGIN_CODE Aivs
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "AI Gen VST"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    VST3_CATEGORIES Instrument Synth
)

# Source files
target_sources(AIGenVST
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/SamplerEngine.cpp
        Source/PitchDetector.cpp
        Source/AIGenerator.cpp
)

# Compile definitions
target_compile_definitions(AIGenVST
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=1
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# Link libraries
target_link_libraries(AIGenVST
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
