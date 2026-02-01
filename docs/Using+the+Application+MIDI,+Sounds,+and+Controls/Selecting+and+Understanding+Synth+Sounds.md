# Using the Application: MIDI, Sounds, and Controls

## Selecting and Understanding Synth Sounds

This section explains how to choose and interpret the various synth “voices” bundled in the application. A global variable, **global_soundnumber**, determines which voice-factory function is used to populate the `PolySynth` instance at startup. Each voice factory is implemented as a Tonic `Synth` and exposes distinct timbral and control behaviors.

---

## Voice Selection Mechanism

When the application launches, it parses command-line arguments (or defaults to 0) to set **global_soundnumber**. During initialization, it calls `poly.addVoices(...)` with the corresponding factory:

```cpp
// Choose voice based on global_soundnumber
if (global_soundnumber <= 0 || global_soundnumber == 1) {
  poly.addVoices(createSynthVoice, 8);      // Basic synth
}
else if (global_soundnumber == 2) {
  poly.addVoices(createSynthVoice_v2, 8);   // Control switcher synth
}
// … continued for voices 3 through 16 …
else if (global_soundnumber >= 16) {
  poly.addVoices(createSynthVoice_v16, 8);  // Sine-sum synth
}
```

Each voice is polyphonic (8 voices) and responds to MIDI note on/off events .

---

## Available Synth Voices

| # | Factory Function | Description | Status |
| --- | --- | --- | --- |
| 1 | createSynthVoice | Basic square+FM synth with LPF envelope | Stable |
| 2 | createSynthVoice_v2 | Control-switcher rhythmic synth | Stable |
| 3 | createSynthVoice_v3 | Arbitrary table-lookup oscillator | Stable |
| 4 | createSynthVoice_v4 | Events buffer-player synth | Under construction |
| 5 | createSynthVoice_v5 | Band-limited oscillator synth | Stable |
| 6 | createSynthVoice_v6 | Compressor synth (dynamics processing) | Stable |
| 7 | createSynthVoice_v7 | Control snap-to-scale synth | Under construction |
| 8 | createSynthVoice_v8 | Delay-effect synth | Stable |
| 9 | createSynthVoice_v9 | Events-based synth | Stable |
| 10 | createSynthVoice_v10 | Filtered noise synth | Stable |
| 11 | createSynthVoice_v11 | Filter synth (HPF/LPF/BPF) | Under construction |
| 12 | createSynthVoice_v12 | FM drone synth | Stable |
| 13 | createSynthVoice_v13 | Low-frequency noise synth | Stable |
| 14 | createSynthVoice_v14 | Reverb effect synth | Stable |
| 15 | createSynthVoice_v15 | Simple step-sequencer synth | Under construction |
| 16 | createSynthVoice_v16 | Sine-sum additive synth | Stable |


---

## 1. Basic Synth

**Function:** createSynthVoice

This voice generates a detuned square wave carrier modulated by a slow sine, with ADSR amplitude and a low-pass filter whose cutoff tracks the note and velocity.

```cpp
Synth createSynthVoice(){
  Synth s;
  auto note = s.addParameter("polyNote", 0.0);
  auto gate = s.addParameter("polyGate", 0.0);
  auto vel  = s.addParameter("polyVelocity", 0.0);
  auto vn   = s.addParameter("polyVoiceNumber", 0.0);
  auto freq = ControlMidiToFreq().input(note) + vn * 1.2;        // slight detune

  auto tone = SquareWave().freq(freq) * SineWave().freq(50);
  auto env  = ADSR().attack(0.04).decay(0.1).sustain(0.8).release(0.6)
                    .doesSustain(true).trigger(gate);
  auto filt = LPF24().cutoff(freq * 0.5 + 200)
                     .Q(1.0 + vel * 0.02);
  auto out  = ((tone * env) >> filt) * (0.02 + vel * 0.005);

  s.setOutputGen(out);
  return s;
}
```

Key points:

- **Detune** by voice number
- **ADSR** for amplitude shaping
- **LPF24** cutoff scaled by note & velocity

---

## 2. Control-Switcher Synth

**Function:** createSynthVoice_v2

Implements a phrase-based sequencer using `ControlSwitcher` and randomized parameters to switch modes, generate click patterns, and FM-style voice mixing.

```cpp
Synth createSynthVoice_v2(){
  Synth s;
  // ... parameter setup ...
  auto metro     = ControlMetro().bpm(s.addParameter("bpm", 320));
  auto modeSw    = ControlMetro().bpm(4);
  auto phraseSt  = ControlRandom().min(0).max(5).trigger(modeSw);
  auto phraseLen = ControlRandom().min(3).max(11).trigger(modeSw);
  auto step      = ControlStepper().start(phraseSt)
                                    .end(phraseSt + phraseLen - 1)
                                    .trigger(metro);

  ControlSwitcher fmAmt(s), sus(s), dec(s), spread(s);
  for(int i=0;i<20;i++){
    fmAmt.addInput(randomFloat(0,10));
    sus.addInput(randomFloat(0,0.7));
    dec.addInput(randomFloat(0.03,0.1));
    spread.addInput(randomFloat(0,0.5));
  }
  // ... synth network using fmAmt, sus, dec, etc. ...
}
```

Highlights:

- **ControlMetro**, **ControlStepper**, **ControlSwitcher** for rhythmic sequencing
- Randomized **FM**, **sustain**, **decay**, **spread** values

---

## 3. Arbitrary Table-Lookup Synth

**Function:** createSynthVoice_v3

Builds a custom `SampleTable` of sinesum partials, feeds it into `TableLookupOsc`, and adds LFO modulation and delay.

```cpp
Synth createSynthVoice_v3(){
  Synth s;
  // Build lookup table
  const unsigned tablesize = 2500;
  SampleTable tbl(tablesize,1);
  TonicFloat norm = 1.0f/tablesize;
  for(unsigned i=0;i<tablesize;i++){
    TonicFloat phase = TWO_PI * i * norm;
    tbl.dataPointer()[i] = 0.75f*sinf(phase)
                         + 0.5f*sinf(2*phase)
                         + 0.25f*sinf(5*phase);
  }
  // Oscillator with LFO
  auto osc = TableLookupOsc().setLookupTable(tbl)
             .freq(ControlMidiToFreq().input(s.addParameter("polyNote",0.0))
                   + s.addParameter("lfo_amp",40.0) 
                     .min(10).max(100)
                   * SineWave()
                     .freq(s.addParameter("lfo_freq",5.0)
                          .min(0.1).max(10.0)));
  // Delay and envelope
  auto delayed = StereoDelay(3.0f,3.0f)
                 .delayTimeLeft(s.addParameter("delay_left",0.5))
                 .delayTimeRight(s.addParameter("delay_right",0.55))
                 .feedback(s.addParameter("delay_feedback",0.4))
                 .wetLevel(s.addParameter("delay_wetlevel",0.5));
  auto env = ADSR().attack(0.04).decay(0.1).sustain(0.8).release(0.6)
                  .doesSustain(true)
                  .trigger(s.addParameter("polyGate",0.0));
  s.setOutputGen((osc * env) >> delayed);
  return s;
}
```

Features:

- **Custom wavetable** construction
- **TableLookupOsc** with parameterized LFO
- **StereoDelay** effect

---

## 6. Compressor Synth

**Function:** createSynthVoice_v6

Applies a dynamic **Compressor** block to a simple tone+noise network. Exposes threshold, ratio, attack, release, makeup-gain, and bypass toggles.

```cpp
Synth createSynthVoice_v6(){
  Synth s;
  // parameters
  auto thr  = s.addParameter("comp_threshold", -12.f)
               .displayName("Threshold (dBFS)")
               .min(-60).max(0);
  auto ratio= s.addParameter("comp_ratio", 1.0f)
               .displayName("Ratio")
               .min(1).max(64).logarithmic(true);
  auto atk  = s.addParameter("comp_attack",0.001f)
               .displayName("Attack (s)")
               .min(0.001).max(0.1).logarithmic(true);
  auto rel  = s.addParameter("comp_release",0.06f)
               .displayName("Release (s)")
               .min(0.01).max(0.08).logarithmic(true);
  auto gain = s.addParameter("comp_gain", 36.f)
               .displayName("Makeup Gain (dBFS)")
               .min(0).max(36);
  auto byp  = s.addParameter("comp_bypass", false)
               .parameterType(ControlParameterTypeToggle);

  // basic tone + noise
  auto freq = ControlMidiToFreq().input(s.addParameter("polyNote",0));
  auto tone = SquareWave().freq(freq) * ADSR().trigger(s.addParameter("polyGate",0));
  auto noise= (Noise() >> HPF24().cutoff(2000))
             * ADSR().trigger(s.addParameter("polyGate",0));

  // compressor block
  auto comp = Compressor()
              .threshold(ControlDbToLinear().input(thr))
              .ratio(ratio)
              .attack(atk)
              .release(rel)
              .makeupGain(ControlDbToLinear().input(gain))
              .bypass(byp);

  s.setOutputGen(((tone + noise) >> comp) * 0.5);
  return s;
}
```

This voice demonstrates dynamic range control integrated into polyphonic voices .

---

## 16. Sine-Sum Additive Synth

**Function:** createSynthVoice_v16

Creates an adder of multiple sine waves spaced by musical intervals around a base pitch.

```cpp
class SineSumSynth : public Synth {
public:
  SineSumSynth(){
    auto pitch = addParameter("pitch", 0);
    Adder adder;
    const int NUM_SINES = 10;
    for(int s=0; s<NUM_SINES; s++){
      auto p = (pitch * 220 + 220)
               * powf(2, (s - (NUM_SINES/2)) * 5.0f / 12.0f);
      adder.input(SineWave().freq(p.smoothed()));
    }
    setOutputGen(adder * ((1.0f/NUM_SINES) * 0.5f));
  }
};
TONIC_REGISTER_SYNTH(SineSumSynth);
```

Ideal for smooth, bell-like textures and additive timbres .

---

## Summary and Usage

- Select **global_soundnumber** at launch (e.g. command-line or UI) to switch voices.
- Experiment with parameters exposed by each voice factory (`synth.setParameter(...)`).
- Combine polyphony with built-in effects (delay, reverb, compressor) for rich, evolving sounds.

Use the table above to guide your choice of voice design. Each factory demonstrates different synthesis paradigms—oscillator-based, wavetable, sequencer-driven, dynamic processing, and additive synthesis—showcasing Tonic’s flexibility in a Win32 MIDI-driven environment.