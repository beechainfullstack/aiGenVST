#!/bin/bash
# Build script for AI Gen VST

set -e  # Exit on error

echo "======================================"
echo "Building AI Gen VST Plugin"
echo "======================================"

# Check if JUCE_PATH is set
if [ -z "$JUCE_PATH" ]; then
    JUCE_PATH="$HOME/Development/JUCE"
    echo "Using default JUCE path: $JUCE_PATH"
fi

if [ ! -d "$JUCE_PATH" ]; then
    echo "ERROR: JUCE not found at $JUCE_PATH"
    echo "Please install JUCE or set JUCE_PATH environment variable"
    exit 1
fi

# Create build directory
mkdir -p build
cd build

# Configure
echo ""
echo "Configuring..."
cmake .. -DJUCE_PATH="$JUCE_PATH" -DCMAKE_BUILD_TYPE=Release

# Build
echo ""
echo "Building..."
cmake --build . --config Release -j$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)

echo ""
echo "======================================"
echo "Build complete!"
echo "======================================"
echo ""
echo "Plugin locations:"
echo "  VST3: ~/Library/Audio/Plug-Ins/VST3/AI Gen VST.vst3"
echo "  AU:   ~/Library/Audio/Plug-Ins/Components/AI Gen VST.component"
echo ""
echo "Next steps:"
echo "  1. Start Python backend: cd python_backend && python server.py"
echo "  2. Load plugin in your DAW"
echo "  3. Generate instruments!"
echo ""
