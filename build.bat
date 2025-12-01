@echo off
REM Build script for AI Gen VST (Windows)

echo ======================================
echo Building AI Gen VST Plugin
echo ======================================

REM Check if JUCE_PATH is set
if not defined JUCE_PATH (
    set JUCE_PATH=C:\Development\JUCE
    echo Using default JUCE path: %JUCE_PATH%
)

if not exist "%JUCE_PATH%" (
    echo ERROR: JUCE not found at %JUCE_PATH%
    echo Please install JUCE or set JUCE_PATH environment variable
    exit /b 1
)

REM Create build directory
if not exist build mkdir build
cd build

REM Configure
echo.
echo Configuring...
cmake .. -DJUCE_PATH="%JUCE_PATH%" -G "Visual Studio 16 2019"

REM Build
echo.
echo Building...
cmake --build . --config Release

echo.
echo ======================================
echo Build complete!
echo ======================================
echo.
echo Plugin location:
echo   VST3: C:\Program Files\Common Files\VST3\AI Gen VST.vst3
echo.
echo Next steps:
echo   1. Start Python backend: cd python_backend ^&^& python server.py
echo   2. Load plugin in your DAW
echo   3. Generate instruments!
echo.

pause
