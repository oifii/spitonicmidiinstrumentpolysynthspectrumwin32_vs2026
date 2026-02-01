# Getting Started with the Polyphonic MIDI Synth Application – Running the Application

This section guides you through launching and running the Polyphonic MIDI Synth Application. You will learn how to connect a MIDI controller or virtual port, initialize audio and synthesis engines, configure voices, and verify that MIDI messages trigger polyphonic audio through PortAudio and the Tonic library.

## Prerequisites

- A **Windows** desktop environment.
- A MIDI controller (e.g., **Alesis Q49**) or a virtual MIDI port.
- Speakers or headphones connected to your audio output.
- The compiled executable or Visual Studio 2026 solution of the project.

## 1. Connect a MIDI Controller

Before running the application:

- Plug in your MIDI keyboard or pad controller via **USB** or **MIDI DIN**.
- Alternatively, install and activate a **virtual MIDI port** (e.g., loopMIDI).
- Ensure Windows recognizes the device; check **Device Manager** ➔ **Sound, video and game controllers** or MIDI utilities.

## 2. Launch the Application

To start the synth:

```bash
begin.bat
```

Or directly run the executable:

```bash
spitonicmidiinstrumentpolysynthspectrumwin32.exe
```

The application opens a simple status window and logs initialization steps to the console.

## 3. Initialize PortAudio 🖥️

On startup, the program initializes the PortAudio engine to manage real‐time audio I/O:

```cpp
mySPIAudioDevice.global_err = Pa_Initialize();
if (mySPIAudioDevice.global_err != paNoError) {
    // Initialization failed
    return 1;
}
```

This call sets up the core PortAudio system; any failure aborts the application .

## 4. Select the Audio Output Device

After initialization, the application prompts you to select or automatically picks the default output device:

```cpp
mySPIAudioDevice.SelectAudioOutputDevice();
```

This routine scans available audio drivers (WASAPI, ASIO, WDM-KS) and configures `global_outputParameters` for streaming .

## 5. Set the Tonic Sample Rate

The Tonic synthesis engine must match the audio stream’s sample rate. By default, PortAudio uses **44 100 Hz**:

```cpp
Tonic::setSampleRate(SAMPLE_RATE);  // SAMPLE_RATE = 44100
```

This synchronizes all internal oscillators and envelopes .

## 6. Configure Polyphonic Synth Voices 🎹

The application supports multiple “sound programs,” each identified by `global_soundnumber`. It allocates **8 voices** of the selected synth:

| Sound Number | Synth Type |
| --- | --- |
| -------------: | ------------------------------------------------------ |
| 1 (or ≤ 0) | **Basic Synth** (square wave + detune) |
| 2 | **Control Switcher Synth** |
| 3 | **Arbitrary Table Lookup Synth** |
| … | … |
| 16 (≥ 16) | **Sine‐Sum Synth** |


```cpp
if (global_soundnumber == 1) {
    poly.addVoices(createSynthVoice, 8);       // basic synth
} else if (global_soundnumber == 2) {
    poly.addVoices(createSynthVoice_v2, 8);    // control switcher
}
// …
else if (global_soundnumber >= 16) {
    poly.addVoices(createSynthVoice_v16, 8);   // sine‐sum synth
}
```

This table helps you choose or script different sounds via command-line arguments .

## 7. Initialize PortMidi and Open MIDI Input 🎵

The MIDI subsystem scans all available input ports, maps your selected device by name, and opens a stream:

```cpp
err = Pm_OpenInput(&global_pPmStreamMIDIIN,
                   global_inputmidideviceid,
                   NULL, 512, NULL, NULL);
Pm_SetFilter(global_pPmStreamMIDIIN, filter);
global_inited = true;
```

- **Pm_CountDevices()** lists ports.
- **global_inputmididevicename** is matched to an index.
- **Pm_SetFilter** suppresses unwanted system messages .

Finally, a PortMidi-driven polling thread begins:

```cpp
Pt_Start(1, receive_poll, 0);
global_active = true;
```

This ensures MIDI messages are captured and handed off for voice allocation .

## 8. Map MIDI Messages to Synth Voices

Within the polling callback, incoming **Note On** and **Note Off** messages are parsed:

- **Note On** with nonzero velocity calls `poly.noteOn(note, velocity)`.
- **Note Off** (or Note On with velocity 0) calls `poly.noteOff(note)`.

This routing uses Tonic’s **PolySynthWithAllocator**, which manages voice stealing and mixing .

## 9. Render Audio with PortAudio Callback

The `renderCallback` function fills each audio buffer:

```cpp
int renderCallback(void* outputBuffer, void*, 
                   unsigned int nBufferFrames,
                   double, RtAudioStreamStatus, void*) {
    synth.fillBufferOfFloats((float*)outputBuffer,
                             nBufferFrames, nChannels);
    return 0;
}
```

PortAudio invokes this callback in a high‐priority thread, streaming the mixed polyphonic audio to your output device .

## 10. Stopping the Application

To exit:

1. Close the status window or press **Ctrl +C** in the console.
2. The application will call `Pm_Terminate()` and `Pa_Terminate()` internally.
3. All audio and MIDI streams shut down cleanly.

---

**⚠️ Note:**

Always verify your MIDI and audio drivers are up to date. ASIO drivers often yield lower latency. Use **loopMIDI** or similar tools if no hardware MIDI is available.