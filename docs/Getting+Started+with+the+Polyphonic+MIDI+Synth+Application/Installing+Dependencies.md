# Getting Started with the Polyphonic MIDI Synth Application

This section guides you through installing and configuring all third-party libraries required to build and run the Polyphonic MIDI Synth application on Windows. We organize dependencies under a `lib-src` directory so that Visual Studio’s include and link paths resolve automatically.

## Dependency Overview

The synth application relies on several open-source libraries. Place each in `lib-src` under the project root and build them with Visual Studio 2026. Once installed, the project’s `.vcxproj` already points to their include and library folders .

| Dependency | Purpose | Include Path | Library / Binary | Notes |
| --- | --- | --- | --- | --- |
| PortAudio 🎵 | Real-time audio I/O (with ASIO support) | `lib-src/portaudio-2021/portaudio_vs2026/include` |  |  |


- `portaudio_x86.lib` / `portaudio_x64.lib`
- `portaudio_x86.dll` / `portaudio_x64.dll` | Copy DLLs into project root and `Debug`/`Release` folders .         |

| PortMidi 🎹 | Cross-platform MIDI I/O | `lib-src/portmidi/pm_common` |
| --- | --- | --- |


`lib-src/portmidi/porttime`                       | `portmidi-dynamic.lib` (x64)

`portmidi_s.lib` (Win32 static)                    | Build dynamic or static as needed .        |

| Tonic 🧬 | C++ synthesis engine | `lib-src/tonic/Tonic-master/src` | `TonicLibVS2026\<Platform>\<Config>\TonicLib.lib` | Clone from https://github.com/thestk/tonic and build the VS2026 solution . |
| --- | --- | --- | --- | --- |
| libsndfile • | Reading/writing WAV and other audio file formats | `lib-src/libsndfile/include` | `libsndfile-1.lib` | Required by FFT wrapper and sample-table loaders . |
| rfftw 🔄 | FFT for real-to-complex transforms | `lib-src/rfftw_vs2026` | `rfftw.lib` | Wraps FFTW; used by `fourier.h` in the project . |
| SPEAR 🌬 | Generating text-partials for sample tables | *(no include)* | `SPEAR_latest_setup.exe` | Download from http://www.klingbeil.com/spear/SPEAR_latest_setup.exe and export `.txt` partials . |


---

## Step 1: Clone and Organize Repositories

Start by creating a `lib-src` folder next to your `.sln` file and cloning each dependency into it.

```bash
cd path/to/spitonicmidiinstrumentpolysynthspectrumwin32_vs2026
mkdir lib-src
cd lib-src

# Audio I/O
git clone https://github.com/PortAudio/portaudio.git portaudio-2021

# MIDI I/O
git clone https://github.com/PortMedia/portmidi.git portmidi

# Synth engine
git clone https://github.com/thestk/tonic.git tonic

# Audio file I/O
git clone https://github.com/libsndfile/libsndfile.git libsndfile

# FFT wrapper
git clone https://github.com/robbmelville/rfftw_vs2026.git rfftw_vs2026
```

---

## Step 2: Build Libraries with Visual Studio 2026

Open each library’s solution in Visual Studio and build **both** Debug and Release configurations for **Win32** and **x64**.

1. **PortAudio**
2. Open `portaudio_vs2026.sln`
3. Ensure the preprocessor definition `PA_USE_ASIO=1` is set for all configurations
4. Build **ALL** projects
5. **PortMidi**
6. Open `portmidi.sln`
7. Build the main `portmidi` project for Win32 and x64
8. **Tonic**
9. Open the delivered `TonicLibVS2026.sln`
10. Build `TonicLib` for each platform/configuration
11. **libsndfile**
12. Open `libsndfile.sln`
13. Build static library for Win32 and x64
14. **rfftw**
15. If a VS2026 solution is provided, open and build
16. Otherwise, add `rfftw.c`/`rfftw.h` to a new static-lib project and compile

---

## Step 3: Copy Runtime DLLs

PortAudio ships as DLLs. After building:

```bash
# From project root
copy .\lib-src\portaudio-2021\portaudio_vs2026\build\msvc\Win32\Debug\portaudio_x86.dll .\
copy .\lib-src\portaudio-2021\portaudio_vs2026\build\msvc\Win32\Debug\portaudio_x86.dll .\Debug\
copy .\lib-src\portaudio-2021\portaudio_vs2026\build\msvc\x64\Debug\portaudio_x64.dll .\
copy .\lib-src\portaudio-2021\portaudio_vs2026\build\msvc\x64\Debug\portaudio_x64.dll .\x64\Debug\
# Repeat for Release DLLs
```

This ensures the application finds them at runtime .

```card
{
    "title": "Enable ASIO",
    "content": "Define PA_USE_ASIO=1 in PortAudio projects to unlock low-latency ASIO support."
}
```

---

## Step 4: (Optional) Generate Partial-Table Data with SPEAR

If you plan to use **partials-based** synth tables:

1. Download and install `SPEAR_latest_setup.exe`
2. Launch SPEAR, **open** a WAV file, then **File → Export Partial Analysis** to produce a `.txt` file
3. Place your partials files in the project folder; the code uses `SpearTextPartialsReader` to ingest them .

---

## Finalizing Setup

1. Reload the **Spitonic MIDI Instrument Synth** solution in Visual Studio
2. Confirm all `AdditionalIncludeDirectories` and `AdditionalDependencies` point to your `lib-src` paths
3. Build **Debug** and **Release**, for both **Win32** and **x64**
4. Run the synthesizer; ensure MIDI devices and audio drivers are available

You are now ready to explore polyphonic MIDI synthesis and spectrum-driven audio tools on Windows!