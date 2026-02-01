# Getting Started with the Polyphonic MIDI Synth Application – Building the Application

This section walks you through compiling the **spitonicmidiinstrument** Win32 project in Visual Studio 2026. You’ll learn how to open the solution, select your target configuration, understand the key dependency settings, and verify a successful build that produces a MIDI-driven polyphonic synthesizer executable.

## Opening the Visual Studio Project

Locate and open the solution or project file in Visual Studio.

- In **File → Open → Project/Solution**, navigate to the repository root.
- Select `spitonicmidiinstrumentsynthwin32.sln` (or directly `spitonicmidiinstrumentsynthwin32.vcxproj`).
- Visual Studio will load the **PolySynth** Win32 application along with its PortAudio, PortMidi, and Tonic dependencies.

## Selecting a Build Configuration 🛠

Choose the desired combination of build mode and platform before compiling:

- On the toolbar, open the **Solution Configurations** dropdown:
- **Debug**: enables debug symbols, no optimization.
- **Release**: optimized build, no debug symbols.
- Open the **Solution Platforms** dropdown:
- **Win32**: 32-bit target.
- **x64**: 64-bit target.

## Building the Solution

> **Tip:** For rapid iteration, start with **Debug|Win32**. Switch to **Release|x64** for performance testing.

1. With configuration and platform set, press **Ctrl + Shift + B** or select **Build → Build Solution**.
2. The Output window will display compilation progress, include-path resolutions, and link steps.
3. Confirm that no errors occur; look for a message similar to:

```plaintext
   ========== Build: 1 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========
```

## Key Project Configuration

The `.vcxproj` file encapsulates all dependency setups:

| Dependency | Include Path / Linker Setting | Purpose |
| --- | --- | --- |
| **PortAudio** | `Additional Include Directories` / `portaudio.lib` | Real-time audio input/output |
| **PortMidi** | `Additional Include Directories` / `portmidi.lib` | MIDI device enumeration and I/O |
| **Tonic** | Sample rate set via `Tonic::setSampleRate(…)` | Polyphonic synth engine configuration |


## How the Build Wires Up the Synth Engine

> The Visual Studio project automatically adds these paths and libraries. Inspect the `<ItemDefinitionGroup>` in your `.vcxproj` under `IncludePaths` and `Linker → AdditionalDependencies` to verify.

Upon startup, the application initializes PortAudio, selects audio devices, sets the Tonic sample rate (commonly **44100 Hz**), and binds a PortAudio callback into the polyphonic synth engine. A minimal excerpt from the initialization routine shows this flow:

```cpp
// Initialize PortAudio
PaError err = Pa_Initialize();
if (err != paNoError) {
    // handle error…
}

// Optionally select input/output devices here…

// Set Tonic synth sample rate (defaults to 44100 if omitted)
Tonic::setSampleRate(SAMPLE_RATE);

// Configure polyphonic voices (e.g., 8 voices of basic synth)
poly.addVoices(createSynthVoice, 8);

// Start the audio stream…
```

This snippet is drawn from the main entry point in `PolySynth_main.cpp` where PortAudio and Tonic are wired together .

## Running the Synth Executable 🎹

After a successful build:

1. In Solution Explorer, right-click **spitonicmidiinstrumentsynthwin32** → **Debug → Start New Instance**.
2. A console window appears and PortAudio opens an audio stream.
3. Plug in a MIDI keyboard or use a virtual MIDI loopback—notes will trigger polyphonic voices in real time.

```card
{
    "title": "Build Verification",
    "content": "Ensure `paNoError` check passes and `Tonic::setSampleRate` reports the expected rate."
}
```

> **Card**

By following these steps, you’ll have a working Windows executable that responds to MIDI input and generates polyphonic synthesized audio. Enjoy exploring different synth voices and creating your own custom patches!