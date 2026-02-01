# Getting Started with the Polyphonic MIDI Synth Application

## System Requirements

Ensure your development machine meets the following minimum specifications before building or running the Polyphonic MIDI Synth Application.

## Supported Platforms 🖥️

- **Operating System**: Windows 10 or later
- **CPU Architecture**: x86 (Win32) and x64

## Development Environment

- **IDE & Toolset**: Microsoft Visual Studio with C++ workload
- Compatible with toolsets **v141–v145** (Visual Studio 2017–2019)
- **Build Configurations**:
- Debug | Win32
- Debug | x64
- Release | Win32
- Release | x64

## Required Libraries

Below is a summary of core dependencies and their roles in the project:

| Dependency | Purpose | Include Path |
| --- | --- | --- |
| **PortAudio** | Cross-platform audio I/O | .\lib-src\portaudio-2021\portaudio_vs2026\include |
| **PortMidi** | MIDI device input/output | .\lib-src\portmidi\pm_common<br/>.\lib-src\portmidi\porttime |
| **Tonic** | Modular synthesis engine | .\lib-src\tonic\Tonic-master\src |
| **libsndfile** | WAV/FLAC file reading & writing | .\lib-src\libsndfile\include |
| **rfftw** | FFT routines for spectrum analysis | .\lib-src\rfftw_vs2026 |


## Optional Dependencies

> Note: The FFT wrapper (`fourier.h`) depends on **libsndfile** for audio I/O.

- **ASIO Drivers**
- For ultra-low latency audio routing.
- Enable by defining `PA_USE_ASIO=1` in your project settings.
- **FreeImage**
- Used for spectrum visualization assets (optional).

## Driver Requirements

- **Audio Interface**:
- Install manufacturer-provided drivers for your USB/firewire sound card.
- **MIDI Controller**:
- Ensure Windows recognizes your MIDI keyboard or pad controller.

## Example Project Configuration

The following snippet illustrates key Visual Studio project settings for Win32 Debug:

```xml
<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
  <PreprocessorDefinitions>
    WIN32;_DEBUG;_WINDOWS;PA_USE_ASIO=1;%(PreprocessorDefinitions)
  </PreprocessorDefinitions>
  <AdditionalIncludeDirectories>
    .\lib-src\portaudio\include;
    .\lib-src\portmidi\pm_common;
    .\lib-src\portmidi\porttime;
    .\lib-src\tonic\Tonic-master\src;
    .\lib-src\libsndfile\include;
    .\lib-src\rfftw_vs2026
  </AdditionalIncludeDirectories>
  <AdditionalDependencies>
    .\lib-src\portaudio\build\msvc\Win32\Debug\portaudio_x86.lib;
    .\lib-src\portmidi\Release\portmidi_s.lib;
    .\lib-src\tonic\Tonic-master\lib\TonicLibVS2026\Debug\TonicLib.lib;
    .\lib-src\libsndfile\libsndfile-1.lib;
    .\lib-src\rfftw_vs2026\Release\rfftw.lib;
    %(AdditionalDependencies)
  </AdditionalDependencies>
</PropertyGroup>
```

This configuration ensures the compiler and linker locate all required headers and libraries.

```card
{
    "title": "Enable ASIO",
    "content": "Defining PA_USE_ASIO significantly reduces audio latency when an ASIO driver is available."
}
```