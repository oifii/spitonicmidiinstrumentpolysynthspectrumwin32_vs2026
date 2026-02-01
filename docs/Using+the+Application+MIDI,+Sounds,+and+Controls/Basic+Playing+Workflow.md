# Using the Application: MIDI, Sounds, and Controls – Basic Playing Workflow 🎹

Playing the PolySynth instrument is as simple as sending MIDI messages from your controller to the application. Behind the scenes, each **Note-On** allocates a voice and triggers its envelope; each **Note-Off** releases it. Voices convert note number and velocity into frequency, filter cutoff, and output level. An effects chain (e.g., stereo delay or reverb) adds final timbral shaping before audio is rendered.

## 1. Workflow Overview

- Send a MIDI **Note-On** or **Note-Off** message on the configured channel.
- **midiCallback** parses the message and calls **poly.noteOn** or **poly.noteOff**.
- **PolySynth** delegates to its **VoiceAllocator** to manage active/inactive voices.
- Allocated voice’s **Synth** has parameters set (`polyNote`, `polyGate`, `polyVelocity`, `polyVoiceNumber`).
- Each voice generator computes frequency, envelope, filter, and output level.
- Voices mix together; global effects (delay, reverb) process the mixed signal.
- **renderCallback** fills the audio buffer via **RtAudio**.

## 2. MIDI Input Handling

The application listens for MIDI input using **RtMidiIn**. Incoming messages route to `midiCallback`, which distinguishes Note-On from Note-Off:

```cpp
void midiCallback(double deltatime, vector<unsigned char>* msg, void* userData){
    int chan     = (*msg)[0] & 0xf;
    int msgtype  = (*msg)[0] & 0xf0;
    int note     = (*msg)[1];
    int velocity = (*msg)[2];

    if (msgtype == 0x80 || (msgtype == 0x90 && velocity == 0)) {
        std::cout << "MIDI Note OFF  C:" << chan << " N:" << note << std::endl;
        poly.noteOff(note);
    }
    else if (msgtype == 0x90) {
        std::cout << "MIDI Note ON   C:" << chan << " N:" << note
                  << " V:" << velocity << std::endl;
        poly.noteOn(note, velocity);
    }
}
```

This callback filters Note-On (0x90) and Note-Off (0x80 or Note-On with zero velocity) messages and invokes the corresponding **PolySynth** methods .

## 3. Voice Allocation & Stealing

**PolySynth** uses a templated allocator (`LowestNoteStealingPolyphonicAllocator` by default) to manage up to N voices:

```cpp
template<typename VoiceAllocator>
class PolySynthWithAllocator : public Synth {
public:
  void addVoices(VoiceCreateFn createFn, int count) {
    for (int i = 0; i < count; i++)
      addVoice(createFn());
  }
  void noteOn(int note, int velocity) {
    allocator.noteOn(note, velocity);
  }
  void noteOff(int note) {
    allocator.noteOff(note);
  }
protected:
  Mixer mixer;
  VoiceAllocator allocator;
};
typedef PolySynthWithAllocator<LowestNoteStealingPolyphonicAllocator> PolySynth;
```

Each voice is a `Synth` added to an inactive queue; on **noteOn**, the allocator picks an available voice or steals one based on policy .

### Allocator Behavior

| Method | Action |
| --- | --- |
| `addVoice` | Adds new voice to **inactive** queue |
| `noteOn` |  |


| `noteOff` |
| --- |


- Selects voice via `getNextVoice(note)`
- Sets `polyNote`, `polyGate` = 1.0, `polyVelocity`, `polyVoiceNumber`
- Moves voice to **active** queue  |
- Finds oldest active voice matching note
- Sets `polyGate` = 0.0
- Moves voice back to **inactive** queue  |

## 4. Synth Voice Definition

Each voice is instantiated by `createSynthVoice()`, defining parameters and generators:

```cpp
Synth createSynthVoice(){
  Synth newSynth;
  auto noteNum      = newSynth.addParameter("polyNote",     0.0);
  auto gate         = newSynth.addParameter("polyGate",     0.0);
  auto velocity     = newSynth.addParameter("polyVelocity", 0.0);
  auto voiceNumber  = newSynth.addParameter("polyVoiceNumber", 0.0);

  // Convert MIDI note to frequency, with slight detune per voice
  auto voiceFreq = ControlMidiToFreq().input(noteNum)
                   + voiceNumber * 1.2;

  // Tone generation and filter
  Generator tone = SquareWave().freq(voiceFreq) * SineWave().freq(50);
  ADSR env = ADSR().attack(0.04).decay(0.1)
               .sustain(0.8).release(0.6)
               .doesSustain(true)
               .trigger(gate);
  auto filterFreq = voiceFreq * 0.5 + 200;
  LPF24 filter = LPF24().Q(1.0 + velocity * 0.02)
                       .cutoff(filterFreq);

  // Output level scales with velocity
  Generator output = ((tone * env) >> filter)
                     * (0.02 + velocity * 0.005);
  newSynth.setOutputGen(output);
  return newSynth;
}
```

This voice responds to **note number** (pitch), **gate** (envelope trigger), and **velocity** (level/filter modulation) .

## 5. Effects Processing

After mixing all voices, a global **StereoDelay** (or other effect) adds depth:

```cpp
poly.addVoices(createSynthVoice, 8);
StereoDelay delay = StereoDelay(3.0f, 3.0f)
  .delayTimeLeft(0.25 + SineWave().freq(0.2) * 0.01)
  .delayTimeRight(0.30 + SineWave().freq(0.23) * 0.01)
  .feedback(0.4)
  .dryLevel(0.8)
  .wetLevel(0.2);
synth.setOutputGen(poly >> delay);
```

The mixed polyphonic output is piped through delay, then sent to the audio callback .

## 6. Audio Rendering

Audio is delivered to the sound card via **RtAudio** in `renderCallback`:

```cpp
int renderCallback(void* outputBuffer, void* /*inputBuffer*/,
                   unsigned int nFrames, double /*time*/,
                   RtAudioStreamStatus /*status*/, void* /*userData*/) {
  // Fill interleaved float buffer: left/right channels
  synth.fillBufferOfFloats((float*)outputBuffer, nFrames, 2);
  return 0;
}
```

This function is registered with RtAudio before starting the stream .

## 7. Workflow Diagram

```mermaid
flowchart LR
  A[Controller] -->|MIDI Note On/Off| B[midiCallback]
  B -->|poly.noteOn/poly.noteOff| C[PolySynth]
  C --> D[VoiceAllocator]
  D --> E[Synth Voice setParameters]
  E --> F[Synth Voice Processing]
  F --> G[Mixer]
  G --> H[Effects Chain]
  H --> I[Audio Output via RtAudio]
```

This flowchart illustrates the path from incoming MIDI to rendered audio.