# Synth Sound Designs (Tonic Voices) - Sine-Sum and Spectral Synths

This section describes **polyphonic**, MIDI-driven Tonic voices that generate rich timbres by summing sine components. Two main families are covered:

- **Sine-Sum Synths**: Combine equal-amplitude sines at harmonic intervals.
- **Spectral (Partials-Based) Synths**: Use SPEAR-extracted partials to recreate real-instrument spectra.

These voices expose control parameters for pitch distribution, envelopes, filtering, and more.

| 🔊 **Synth Class / Factory** | **Type** | **Timbre Generation** | **Key Controls** |
| --- | --- | --- | --- |
| SimpleInstrumentSineSumSynth | Synth subclass | Sum of NUM_SINES sines around MIDI note | `pitch`, `trigger`, `midiNoteVelocity` |
| createSynthVoice_v16 | Factory function | Detuned sine-sum with `pitchmulti` & `pitchbase` | `pitchmulti`, `pitchbase`, `polyVelocity` |
| SimpleInstrumentTableLookupSPEARSynth | Synth subclass | Sine sum weighted by SPEAR partial amplitudes | SPEAR file path, amplitude scaling |


---

## SimpleInstrumentSineSumSynth

This class builds a classic sine-sum synth that blends several harmonics into one voice. It demonstrates how to use an **Adder** to aggregate SineWave generators and normalize their combined output.

- Defines `NUM_SINES` constant (default 10).
- Creates a **ControlGenerator** for MIDI note → frequency.
- Adds a **ControlParameter** `pitch` to shift the harmonic spacing.
- Loops to spawn `NUM_SINES` sine oscillators at scaled semitone offsets.
- Normalizes the sum by `1/NUM_SINES` and passes it through an ADSR envelope.

```cpp
#define NUM_SINES 10
class SimpleInstrumentSineSumSynth : public Synth {
public:
  SimpleInstrumentSineSumSynth(){
    auto midiNote  = addParameter("midiNote");
    auto velocity  = addParameter("midiNoteVelocity");
    auto noteFreq  = ControlMidiToFreq().input(midiNote);
    auto trigger   = addParameter("trigger");
    auto pitch     = addParameter("pitch", 0.0f);
    Adder outputAdder;
    for(int s=0; s<NUM_SINES; s++){
      auto ratio = powf(2.0f, (s-(NUM_SINES/2))*5.0f/12.0f);
      auto genFreq = (pitch * noteFreq + noteFreq) * ratio;
      outputAdder.input(SineWave().freq(genFreq.smoothed()));
    }
    auto osc    = outputAdder * (1.0f/NUM_SINES);
    auto voiced = osc * ADSR()
                      .attack(0.01).decay(1.5).sustain(0.0).release(0.0)
                      .legato(true).trigger(trigger);
    setOutputGen(voiced);
  }
};
TONIC_REGISTER_SYNTH(SimpleInstrumentSineSumSynth);
```

This implementation highlights use of the **Adder** pattern and **ControlParameter** smoothing for stable frequency transitions .

---

## createSynthVoice_v16 (Enhanced Sine-Sum)

The `createSynthVoice_v16` factory function extends the basic sine-sum by adding:

- **Voice-level detune** via `voiceNumber`.
- `**pitchmulti**` parameter to dynamically control harmonic spacing around the base MIDI frequency.
- `**pitchbase**` parameter to set a fixed reference frequency.
- A **low-pass filter** (`LPF24`) modulated by velocity.

```cpp
Synth createSynthVoice_v16(){
  Synth newSynth;
  auto noteNum      = newSynth.addParameter("polyNote",    0.0);
  auto gate         = newSynth.addParameter("polyGate",    0.0);
  auto velocity     = newSynth.addParameter("polyVelocity",0.0);
  auto voiceNumber  = newSynth.addParameter("polyVoiceNumber",0.0);
  auto voiceFreq    = ControlMidiToFreq().input(noteNum) + voiceNumber*1.2;

  const int NUM_SINES = 10;
  auto pitchmulti    = newSynth.addParameter("pitchmulti", 0.0).min(0).max(10);
  auto pitchbase     = newSynth.addParameter("pitchbase", 110.0).min(20).max(10000);
  Adder outputAdder;
  for (int s = 0; s < NUM_SINES; s++) {
    float semitoneOffset = (s - NUM_SINES/2) * (5.0f/12.0f);
    auto genFreq = (pitchmulti*voiceFreq + voiceFreq) * powf(2, semitoneOffset);
    outputAdder.input(SineWave().freq(genFreq.smoothed()));
  }
  auto tone = outputAdder * ((1.0f/NUM_SINES)*2.0f);
  auto env  = ADSR().attack(0.04).decay(0.1).sustain(0.8).release(0.6)
                    .doesSustain(true).trigger(gate);
  auto filterFreq = voiceFreq*0.5 + 200;
  auto filt       = LPF24().Q(1.0 + velocity*0.02).cutoff(filterFreq);
  auto output     = (tone * env) >> filt * (0.02 + velocity*0.005);
  newSynth.setOutputGen(output);
  return newSynth;
}
```

This voice leverages **per-voice detune**, **normalized sum** and **filtering** to sculpt evolving harmonic textures .

---

## Spectral Synths (SPEAR Partials)

Spectral synth variants load partial-data exported from SPEAR to recreate real-instrument spectra. They perform:

1. **Reading** SPEAR text file via `SpearTextPartialsReader`.
2. **Normalizing** each partial frequency to a base MIDI note.
3. **Generating** a sine oscillator per partial, weighted by its amplitude.
4. **Summing** oscillators in an **Adder**.
5. **Applying** envelope and optional filtering/delay.

```cpp
#include "speartextpartialsreader.h"
class SimpleInstrumentTableLookupSPEARSynth : public Synth {
public:
  SimpleInstrumentTableLookupSPEARSynth(){
    // 1) Load SPEAR partials for B3 (246.9 Hz)
    SpearTextPartialsReader reader("VC-8_mV_sus-sh_mf_B3(partials).txt");
    // 2) Normalize to base frequency
    for (auto &freq : reader.partialfrequencies) {
      freq /= 246.9f;
    }
    // 3) Build sine-sum from each partial
    Adder spectralAdder;
    for (size_t i=0; i<reader.partialfrequencies.size(); ++i) {
      float f = reader.partialfrequencies[i] * ControlMidiToFreq().input(
                  addParameter("midiNote")
                ).smoothed();
      float amp = reader.partialAmplitudes[i]; // amplitude from SPEAR
      spectralAdder.input(SineWave().freq(f) * amp);
    }
    // 4) Envelope + output
    auto env = ADSR().attack(0.01).decay(0.1).sustain(0.8).release(0.5)
                     .trigger(addParameter("trigger"));
    setOutputGen(spectralAdder * env * (1.0f / reader.partialfrequencies.size()));
  }
};
TONIC_REGISTER_SYNTH(SimpleInstrumentTableLookupSPEARSynth);
```

This approach tightly matches real-world spectra by mirroring partial frequencies and amplitudes extracted from recordings .

---

**Key Takeaways**

```card
{
    "title": "Normalize Output",
    "content": "Always divide the Adder sum by NUM_SINES (or number of partials) to prevent amplitude overload."
}
```