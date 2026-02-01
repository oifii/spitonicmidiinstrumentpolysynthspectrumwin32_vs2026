# Audio Device and MIDI Device Configuration – ASIO-Specific Configuration

This section explains how the synthesizer configures audio and MIDI devices, with special handling when an ASIO host API is detected. ASIO is preferred for **low-latency** performance. The code uses `PaAsioStreamInfo` to bind specific channel selectors, ensuring audio streams use the correct input/output channels on multi-channel ASIO devices.

## Overview

- The application maintains global arrays for channel selectors:
- `global_inputAudioChannelSelectors[2]`
- `global_outputAudioChannelSelectors[2]`
- When an ASIO device is selected, these arrays specify which channels to use.
- If the chosen device is not ASIO or is the default device, `hostApiSpecificStreamInfo` is set to `NULL`, and PortAudio falls back to standard behavior.

## Key Components

| Component | Purpose |
| --- | --- |
| **global_inputAudioChannelSelectors** 🎚️ | Specifies the two input channels for ASIO devices |
| **global_outputAudioChannelSelectors** 🎚️ | Specifies the two output channels for ASIO devices |
| **PaAsioStreamInfo** | ASIO-specific structure passed to PortAudio |
| **hostApiSpecificStreamInfo** | Pointer in `PaStreamParameters` that directs PortAudio on host-API behavior |
| **SelectAudioInputDevice() / SelectAudioOutputDevice()** | Functions that set up `PaStreamParameters`, including ASIO config when applicable |


## ASIO Configuration Structure

```cpp
// WARNING – this is not portable across host APIs.
global_asioInputInfo.size            = sizeof(PaAsioStreamInfo);
global_asioInputInfo.hostApiType     = paASIO;
global_asioInputInfo.version         = 1;
global_asioInputInfo.flags           = paAsioUseChannelSelectors;
global_asioInputInfo.channelSelectors = global_inputAudioChannelSelectors;
```

```cpp
global_asioOutputInfo.size            = sizeof(PaAsioStreamInfo);
global_asioOutputInfo.hostApiType     = paASIO;
global_asioOutputInfo.version         = 1;
global_asioOutputInfo.flags           = paAsioUseChannelSelectors;
global_asioOutputInfo.channelSelectors = global_outputAudioChannelSelectors;
```

(Declarations in `spiaudiodevice.h`)

## SelectAudioInputDevice()

1. **Scan and map** all available audio devices.
2. **Choose** device by name or default.
3. **Fill** `global_inputParameters`:
4. `device`, `channelCount`, `sampleFormat`, `suggestedLatency`
5. **Configure ASIO**:
6. If device == default → `hostApiSpecificStreamInfo = NULL`
7. Else if host API == `paASIO` → point to `global_asioInputInfo`
8. Else → `hostApiSpecificStreamInfo = NULL`

```cpp
if (deviceid == Pa_GetDefaultInputDevice()) {
    global_inputParameters.hostApiSpecificStreamInfo = NULL;
} else if (Pa_GetHostApiInfo(Pa_GetDeviceInfo(deviceid)->hostApi)->type == paASIO) {
    global_inputParameters.hostApiSpecificStreamInfo = &global_asioInputInfo;
} else {
    global_inputParameters.hostApiSpecificStreamInfo = NULL;
}
```

(Code excerpt from `SelectAudioInputDevice`)

## SelectAudioOutputDevice()

1. **Scan and map** devices (if not already).
2. **Find** the user-selected device or fallback to default.
3. **Populate** `global_outputParameters` similarly.
4. **Assign** ASIO info using the same logic as input.

```cpp
if (deviceid == Pa_GetDefaultOutputDevice()) {
    global_outputParameters.hostApiSpecificStreamInfo = NULL;
}
else if (Pa_GetHostApiInfo(Pa_GetDeviceInfo(deviceid)->hostApi)->type == paASIO) {
    global_outputParameters.hostApiSpecificStreamInfo = &global_asioOutputInfo;
}
else {
    global_outputParameters.hostApiSpecificStreamInfo = NULL;
}
```

(Code excerpt from `SelectAudioOutputDevice`)

## Channel Selector Arrays

The two-element arrays must reference valid channels on the target ASIO device:

| Array | Description |
| --- | --- |
| **global_inputAudioChannelSelectors** | e.g., `{0,1}` for first two ASIO input channels |
| **global_outputAudioChannelSelectors** | e.g., `{0,1}`, or `{2,3}` for left/right on 2nd stereo bus |


## Why ASIO?

> **Note:** Always verify `selectors[i] < deviceInfo->maxInputChannels` or `maxOutputChannels` to avoid invalid channel indices.

- **Low Latency**: Direct kernel-level streaming bypasses layers.
- **Multi-Channel**: Fine-grained control of device channels via `PaAsioStreamInfo`.
- **Professional Audio**: ASIO is standard in digital audio workstations.

## Best Practices

- **Consistency**: Keep selector arrays in sync with device capabilities.
- **Fallback Handling**: Always set `hostApiSpecificStreamInfo = NULL` for non-ASIO devices.
- **Portability**: Guard ASIO code paths; use preprocessor checks if compiling on non-Windows platforms.

```cpp
// Example check (in case of cross-platform builds)
#if PA_USE_ASIO
    // ASIO-specific code
#endif
```

---

With this configuration, the synthesizer ensures it leverages ASIO when available, but seamlessly falls back to PortAudio’s standard behavior otherwise, maintaining both **performance** and **compatibility**.