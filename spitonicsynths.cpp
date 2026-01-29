/*
 * Copyright (c) 2012-2026 Stephane Poirier
 *
 * stephane.poirier@oifii.org
 *
 * Stephane Poirier
 * 1901 rue Gilford, #53
 * Montreal, QC, H2H 1G8
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"

#include "Tonic.h"
#include "sndfile.hh"
#include <assert.h>

#include "spitonicsynths.h"

using namespace Tonic;



/////////////////////////////////////
//based on the simple instrument synth
/////////////////////////////////////
Synth createSynthVoice()
{
	Synth newSynth;

	ControlParameter noteNum = newSynth.addParameter("polyNote", 0.0);
	ControlParameter gate = newSynth.addParameter("polyGate", 0.0);
	ControlParameter noteVelocity = newSynth.addParameter("polyVelocity", 0.0);
	ControlParameter voiceNumber = newSynth.addParameter("polyVoiceNumber", 0.0);

	ControlGenerator voiceFreq = ControlMidiToFreq().input(noteNum) + voiceNumber * 1.2; // detune the voices slightly

	Generator tone = SquareWave().freq(voiceFreq) * SineWave().freq(50);

	ADSR env = ADSR()
		.attack(0.04)
		.decay(0.1)
		.sustain(0.8)
		.release(0.6)
		.doesSustain(true)
		.trigger(gate);

	ControlGenerator filterFreq = voiceFreq * 0.5 + 200;

	LPF24 filter = LPF24().Q(1.0 + noteVelocity * 0.02).cutoff(filterFreq);

	Generator output = ((tone * env) >> filter) * (0.02 + noteVelocity * 0.005);

	newSynth.setOutputGen(output);

	return newSynth;
}




/////////////////////////////////////
//based on the control switcher synth
/////////////////////////////////////
Synth createSynthVoice_v2()
{
	Synth newSynth;

	ControlParameter noteNum = newSynth.addParameter("polyNote", 0.0);
	ControlParameter gate = newSynth.addParameter("polyGate", 0.0);
	ControlParameter noteVelocity = newSynth.addParameter("polyVelocity", 0.0);
	ControlParameter voiceNumber = newSynth.addParameter("polyVoiceNumber", 0.0);

	ControlGenerator voiceFreq = ControlMidiToFreq().input(noteNum) + voiceNumber * 1.2; // detune the voices slightly


	//2023feb19, spi, begin
	//Generator tone = SquareWave().freq(voiceFreq) * SineWave().freq(50);
	//spi, begin
	//ControlMetro metro = ControlMetro().bpm(80 * 4); //original
	ControlParameter bpmparam = newSynth.addParameter("bpm", 320).min(4).max(1000);
	ControlMetro metro = ControlMetro().bpm(bpmparam);
	//spi, end
	ControlGenerator modeSwitch = ControlMetro().bpm(4);
	//ControlGenerator modeSwitch = ControlMetro().bpm(4 * 4);

	const int numSteps = 20;
	//const int numSteps = 5;
	//const int numSteps = 100;

	ControlGenerator phraseStart = ControlRandom().min(0).max(5).trigger(modeSwitch);

	ControlGenerator phraseLen = ControlRandom().min(3).max(11).trigger(modeSwitch);

	ControlStepper step = ControlStepper()
		.start(phraseStart)
		.end(phraseStart + phraseLen - 1)
		.trigger(metro);

	ControlGenerator clickVol = newSynth.addParameter("clickVol"); //newSynth.addParameter("clickVol");
	ADSR clickEnv = ADSR(0, 0.01, 0, 0).trigger(metro);
	Generator clickIntensity = (1 + clickVol).smoothed();
	//Generator click = SineWave().freq(300 + 1000 * clickEnv) * clickEnv * 0.03; //original
	Generator click = SquareWave().freq(300 + 1000 * clickEnv) * clickEnv * 0.03; //spi + cwb
	//Generator click = SineWave().freq(300 + 1000 * clickEnv) * clickEnv * 10;

	ControlSwitcher fmAmount = ControlSwitcher().inputIndex(step);
	ControlSwitcher sustain = ControlSwitcher().inputIndex(step);
	ControlSwitcher decay = ControlSwitcher().inputIndex(step);
	ControlSwitcher spreadSeq = ControlSwitcher().inputIndex(step);

	for (int i = 0; i < numSteps; i++){
		fmAmount.addInput(randomFloat(0, 10));
		sustain.addInput(randomFloat(0, 0.7));
		decay.addInput(randomFloat(0.03, 0.1));
		spreadSeq.addInput(randomFloat(0, 0.5));
	}
	//spi, begin
	ControlParameter freqparam = newSynth.addParameter("freq", 50).min(10).max(1000);
	//Generator freq = ControlValue(50).smoothed(); //original
	Generator freq = freqparam.smoothed();
	//spi, end
	Generator tremelo = 1 + (SineWave().freq(15) *  ADSR(0, 0.5, 0, 0).trigger(modeSwitch));
	Generator bassEnv = ADSR(0.001, 0.1, 0, 0).decay(decay).legato(true).sustain(sustain * sustain).trigger(metro);
	//Generator bassEnv = ADSR(0.1, 0.1, 0, 0).decay(decay).legato(true).sustain(sustain * sustain).trigger(metro);
	ControlGenerator spread = ControlRandom().min(0).max(0.5).trigger(modeSwitch);// * spreadSeq;
	ControlGenerator wave = ControlRandom().min(0.4).max(0.9).trigger(modeSwitch);

	auto makeBass = [&](Generator bassFreq){
		return RectWave()
			.pwm(wave + 0.04 * bassEnv)
			.freq(
			bassFreq
			+ bassFreq
			* SineWave() //original
			//* SawtoothWave() //spi
			//* RectWave() //spi
			.freq(bassFreq * 1.99)
			* (
			fmAmount
			* 0.7
			+ newSynth.addParameter("addtlFM")
			).smoothed()
			);
	};

	Generator bass =
		(
		makeBass(voiceFreq.smoothed())//makeBass(freq)
		>> MonoToStereoPanner().pan(-1 * spread)
		)
		+
		(
		makeBass((voiceFreq.smoothed())*1.02) //makeBass(freq * 1.02) //original
		//makeBass(freq * 1.10) //spi
		>> MonoToStereoPanner().pan(1 * spread)
		)

		//>> HPF12().cutoff(addParameter("hpf") * 1000)
		>> LPF12().cutoff(5500) //original
		//>> LPF12().cutoff(11000)
		>> StereoDelay(1.1, 1.2).wetLevel(0.1); //original
	//>> StereoDelay(1.1, 1.2).wetLevel(0.8); //spi
	//Generator bassWithAmp = bass * bassEnv * tremelo;
	//Generator tone = (bass * bassEnv * tremelo); // +click; //Generator tone = bass * bassEnv * tremelo 
	Generator tone = (bass * bassEnv * tremelo); //spi + cwb 

	//2023feb19, spi, end

	ADSR env = ADSR()
		.attack(0.04)
		.decay(0.1)
		.sustain(0.8)
		.release(0.6)
		.doesSustain(true)
		.trigger(gate);

	ControlGenerator filterFreq = voiceFreq * 0.5 + 200;

	LPF24 filter = LPF24().Q(1.0 + noteVelocity * 0.02).cutoff(filterFreq);

	//2023feb19, spi, begin
	//Generator output = ((tone * env) >> filter) * (0.02 + noteVelocity * 0.005);
	//Generator output = ((tone * env)) * (0.02 + noteVelocity * 0.005) +(click); //works!
	//Generator output = ((tone + click)* env) * (0.02 + noteVelocity * 0.005); //sp;i + cwb
	Generator output = ((tone * env)) * (0.02 + noteVelocity * 0.005); //original
	//2023feb19, spi, end

	newSynth.setOutputGen(output);

	return newSynth;
}




/////////////////////////////////////
//based on the arbitrary table lookup synth
/////////////////////////////////////
Synth createSynthVoice_v3()
{
	Synth newSynth;

	ControlParameter noteNum = newSynth.addParameter("polyNote", 0.0);
	ControlParameter gate = newSynth.addParameter("polyGate", 0.0);
	ControlParameter noteVelocity = newSynth.addParameter("polyVelocity", 0.0);
	ControlParameter voiceNumber = newSynth.addParameter("polyVoiceNumber", 0.0);

	ControlGenerator voiceFreq = ControlMidiToFreq().input(noteNum) + voiceNumber * 1.2; // detune the voices slightly

	//2023feb20, spi, begin
	//Generator tone = SquareWave().freq(voiceFreq) * SineWave().freq(50);

	// Build a table of arbitrary length. TableLookupOsc requires power-of-two+1 length tables, but they will be resized if necessary.
	// Try changing this to 2049 and compare the sound. Should be perceptually almost identical.
	const unsigned int tablesize = 2500;

	// You can register this in a collection if you want to use it in a few oscillators
	SampleTable lookupTable = SampleTable(tablesize, 1);

	TonicFloat norm = 1.0f / tablesize;
	TonicFloat * tableData = lookupTable.dataPointer();
	for (unsigned int i = 0; i<tablesize; i++)
	{
		// sum a few sine waves
		TonicFloat phase = TWO_PI * i * norm;
		*tableData++ = 0.75f * sinf(phase) + 0.5f * sinf(phase * 2) + 0.25f * sinf(phase * 5);
	}

	//TableLookupOsc osc = TableLookupOsc().setLookupTable(lookupTable).freq(100 + 40 * SineWave().freq(0.1)); //original
	//TableLookupOsc osc = TableLookupOsc().setLookupTable(lookupTable).freq(newSynth.addParameter("base_freq", 100.0).min(50.0).max(1000.0) + newSynth.addParameter("lfo_amp", 40.0).min(10.0).max(100.0) * SineWave().freq(newSynth.addParameter("lfo_freq", 5.0).min(0.1).max(10.0))); //spi
	TableLookupOsc osc = TableLookupOsc().setLookupTable(lookupTable).freq(voiceFreq + newSynth.addParameter("lfo_amp", 40.0).min(10.0).max(100.0) * SineWave().freq(newSynth.addParameter("lfo_freq", 5.0).min(0.1).max(10.0))); //spi
	//TableLookupOsc osc = TableLookupOsc().setLookupTable(lookupTable).freq(90 + 30 * SineWave().freq(10)); //spi

	StereoDelay delay = StereoDelay(3.0f, 3.0f, 3.0f, 3.0f)
		//.delayTimeLeft(0.5 + SineWave().freq(0.2) * 0.01)
		.delayTimeLeft(newSynth.addParameter("delay_left", 0.5).min(0.0).max(3.0) + SineWave().freq(0.2) * 0.01)
		//.delayTimeRight(0.55 + SineWave().freq(0.23) * 0.01)
		.delayTimeRight(newSynth.addParameter("delay_right", 0.55).min(0.0).max(3.0) + SineWave().freq(0.23) * 0.01)
		//.feedback(0.3)
		.feedback(newSynth.addParameter("delay_feedback", 0.4).min(0.0).max(1.0))
		.dryLevel(newSynth.addParameter("delay_drylevel", 0.5).min(0.0).max(1.0))
		//.wetLevel(0.2);
		.wetLevel(newSynth.addParameter("delay_wetlevel", 0.5).min(0.0).max(1.0));

	Generator tone = delay.input(osc * newSynth.addParameter("base_amp", 0.80).min(0.0).max(1.0));
	//2023feb20, spi, end

	ADSR env = ADSR()
		.attack(0.04)
		.decay(0.1)
		.sustain(0.8)
		.release(0.6)
		.doesSustain(true)
		.trigger(gate);

	ControlGenerator filterFreq = voiceFreq * 0.5 + 200;

	LPF24 filter = LPF24().Q(1.0 + noteVelocity * 0.02).cutoff(filterFreq);

	//Generator output = ((tone * env) >> filter) * (0.02 + noteVelocity * 0.005); //original
	Generator output = ((tone * env)) * (0.02 + noteVelocity * 0.005); //spi

	newSynth.setOutputGen(output);

	return newSynth;
}




/////////////////////////////////////
//based on the events bufferplayer synth
//todo 2023: use voiceFreq
/////////////////////////////////////
Synth createSynthVoice_v4()
{
	Synth newSynth;

	ControlParameter noteNum = newSynth.addParameter("polyNote", 0.0);
	ControlParameter gate = newSynth.addParameter("polyGate", 0.0);
	ControlParameter noteVelocity = newSynth.addParameter("polyVelocity", 0.0);
	ControlParameter voiceNumber = newSynth.addParameter("polyVoiceNumber", 0.0);

	ControlGenerator voiceFreq = ControlMidiToFreq().input(noteNum) + voiceNumber * 1.2; // detune the voices slightly

	//2023feb20, spi, begin
	//Generator tone = SquareWave().freq(voiceFreq) * SineWave().freq(50);
	const int NUM_VOICES = 5;
	Generator allVoices;

	Generator noise = PinkNoise(); //original
	//Generator noise = 0.5* SineWave().freq(110) + 0.5* SineWave().freq(220); //spi
	//Generator noise = SineWave().freq(220); //spi
	//Generator noise = SawtoothWave().freq(220); //spi
	//Generator noise = LFNoise().setFreq(220); //spi

	//1) load a sample file
	//SampleTable buffer = loadAudioFile("",2);
	SndfileHandle file1;
	//file1 = SndfileHandle("D:\\Program Files\\Native Instruments\\Sample Libraries\\Kontakt 3 Library\\Orchestral\\Z - Samples\\03 Cello ensemble - 8\\VC-8_trem_RS_ff\\VC-8_trem_RS_ff_B3.wav");
	//file1 = SndfileHandle("D:\\Program Files\\Native Instruments\\Sample Libraries\\Kontakt 3 Library\\Band\\Z - Samples\\1 - Horns Samples\\Alto Saxophone Samples\\ASaxA_sus_a#2_3.wav");
	file1 = SndfileHandle("D:\\Program Files\\Native Instruments\\Sample Libraries\\Kontakt 3 Library\\World\\Z - Samples\\Highland Pipes Samples\\HPdrones\\HPdrones.wav");
	assert(file1.samplerate() == 44100);
	assert(file1.channels() == 2);
	float file1duration_s = ((float)file1.frames()) / ((float)file1.samplerate());
	SampleTable buffer1(file1.frames(), file1.channels());
	file1.read(buffer1.dataPointer(), file1.frames()*file1.channels());
	BufferPlayer bPlayer1;
	bPlayer1.setBuffer(buffer1).loop(false);


	vector<float> scale;
	scale.push_back(0);
	scale.push_back(2);
	scale.push_back(4);
	scale.push_back(7);
	scale.push_back(11);

	for (int i = 0; i < NUM_VOICES; i++)
	{
		////////////////////////////////////////////
		// Create the synthesis network
		////////////////////////////////////////////

		Generator voice;

		// Send a trigger message immediately, don't wait for the first beat to happen with the metrenome 
		ControlTrigger initialTrigger;
		initialTrigger.trigger();
		//ControlGenerator resetTrigger = initialTrigger + ControlMetro().bpm(ofRandom(10, 15)); //original - does not compile
		ControlGenerator resetTrigger = initialTrigger + ControlMetro().bpm(ControlRandom().min(10).max(15));

		ControlGenerator noiseTrigger = ControlMetro().bpm(ControlRandom().min(50).max(200).trigger(resetTrigger));

		ControlGenerator pulseLen = ControlRandom().min(0.1).max(0.5).trigger(resetTrigger);

		//ControlGenerator pulse = ControlPulse().length(pulseLen).input(noiseTrigger); //original - does not compile
		ControlGenerator pulse = ControlPulse().length(pulseLen).trigger(noiseTrigger);

		Generator env = ADSR(0.01, 0, 0.5, 0.01).decay(pulseLen * 0.5).trigger(pulse); //original
		//Generator env = ADSR(0.01, 0, 0.05, 0.01).decay(pulseLen * 0.5).trigger(pulse);

		Generator tone = bPlayer1.trigger(pulse);

		//voice = noise * env; //original
		voice = tone * env; //spi
		//voice = noise ;

		ControlGenerator highPassAmount = ControlRandom().min(0).max(1).trigger(resetTrigger);

		ControlSnapToScale scaleSnapper = ControlSnapToScale().setScale(scale);
		scaleSnapper.input(40 + highPassAmount * 50); //original
		//scaleSnapper.input(10 + highPassAmount * 50); //spi
		ControlGenerator filterFreq = ControlMidiToFreq().input(scaleSnapper); //original
		//ControlGenerator filterFreq = ControlMidiToFreq().input(40 + highPassAmount * 50); //spi, bypass snap to scale
		ControlGenerator q = ControlRandom().min(0.5).max(1).trigger(resetTrigger);
		q = q * q * 50;

		//todo: voice should be a function of the frequency (variable voiceFreq)
		//voice = HPF24().cutoff(filterFreq).input(voice).Q(q); //original
		//voice = LPF24().cutoff(filterFreq).input(voice).Q(q); //spi
		//voice = BPF24().cutoff(filterFreq).input(voice).Q(0.1); //spi 2023
		//voice = BPF24().cutoff(voiceFreq).input(voice).Q(q) + LPF24().cutoff(filterFreq).input(voice).Q(q); //spi 2023
		//voice = BPF24().cutoff(voiceFreq).input(voice).Q(q) + 0.5* LPF24().cutoff(filterFreq).input(voice).Q(q); //spi 2023
		voice = 4* BPF24().cutoff(voiceFreq).input(voice).Q(0.1); //spi 2023

		float pan = (2 * (float)i / NUM_VOICES) - 1;
		voice = MonoToStereoPanner().pan(pan).input(voice) * (15.0 / NUM_VOICES);

		allVoices = allVoices + voice;
	}

	Generator allvoicesPlusReverb = Reverb().input(allVoices).density(1).dryLevel(0).wetLevel(0.2).decayTime(1); //original
	//Generator allvoicesPlusReverb = Reverb().input(allVoices).density(1).dryLevel(0).wetLevel(0.9).decayTime(1); //spi

	Generator tone = allVoices + allvoicesPlusReverb;
	//2023feb20, spi, end

	ADSR env = ADSR()
		.attack(0.04)
		.decay(0.1)
		.sustain(0.8)
		.release(0.6)
		.doesSustain(true)
		.trigger(gate);

	ControlGenerator filterFreq = voiceFreq * 0.5 + 200;

	LPF24 filter = LPF24().Q(1.0 + noteVelocity * 0.02).cutoff(filterFreq);

	//Generator output = ((tone * env) >> filter) * (0.02 + noteVelocity * 0.005);
	Generator output = ((tone * env)) * (0.02 + noteVelocity * 0.005);

	newSynth.setOutputGen(output);

	return newSynth;
}




///////////////////////////
//based on the band limited oscillator synth
///////////////////////////
Synth createSynthVoice_v5()
{
	Synth newSynth;

	ControlParameter noteNum = newSynth.addParameter("polyNote", 0.0);
	ControlParameter gate = newSynth.addParameter("polyGate", 0.0);
	ControlParameter noteVelocity = newSynth.addParameter("polyVelocity", 0.0);
	ControlParameter voiceNumber = newSynth.addParameter("polyVoiceNumber", 0.0);

	ControlGenerator voiceFreq = ControlMidiToFreq().input(noteNum) + voiceNumber * 1.2; // detune the voices slightly

	//2023feb21, spi, begin
	//Generator tone = SquareWave().freq(voiceFreq) * SineWave().freq(50);
	//ControlParameter blend = addParameter("blend").min(0).max(1);
	ControlParameter blend = newSynth.addParameter("blend", 0.1f).min(0).max(1);

	//Generator freqSweep = SineWave().freq(0.25) * 1000 + 1080; //original
	//Generator freqSweep = SineWave().freq(1) * 50 + 100;
	Generator freqSweep = SineWave().freq(newSynth.addParameter("lfo_freq", 5.0).min(0.1).max(10.0)) * newSynth.addParameter("lfo_amp", 50.0).min(10.0).max(400.0) + newSynth.addParameter("sweep_freq", 100.0).min(10.0).max(400.0);
	//Generator smoothBlend = blend.smoothed(); //original
	Generator smoothBlend = blend.smoothed(.05);

	//Generator output = (SquareWave().freq(freqSweep) * (1.0 - smoothBlend)) + (SquareWaveBL().freq(freqSweep) * smoothBlend); //original
	//Generator output = (SineWave().freq(freqSweep) * (1.0 - smoothBlend)) + (SineWave().freq(freqSweep) * smoothBlend);
	//Generator output = (TriangleWave().freq(freqSweep) * (1.0 - smoothBlend)) + (SquareWaveBL().freq(freqSweep) * smoothBlend);
	//Generator output = (SquareWave().freq(newSynth.addParameter("base_freq", 80.0).min(10.0).max(400.0)) * (1.0 - smoothBlend)) + (SquareWave().freq(freqSweep) * smoothBlend);
	Generator output = (SquareWave().freq(voiceFreq) * (1.0 - smoothBlend)) + (SquareWave().freq(freqSweep) * smoothBlend);

	StereoDelay delay = StereoDelay(3.0f, 3.0f, 3.0f, 3.0f)
		//.delayTimeLeft(0.5 + SineWave().freq(0.2) * 0.01)
		.delayTimeLeft(newSynth.addParameter("delay_left", 0.5).min(0.0).max(3.0) + SineWave().freq(0.2) * 0.01)
		//.delayTimeRight(0.55 + SineWave().freq(0.23) * 0.01)
		.delayTimeRight(newSynth.addParameter("delay_right", 0.55).min(0.0).max(3.0) + SineWave().freq(0.23) * 0.01)
		//.feedback(0.3)
		.feedback(newSynth.addParameter("delay_feedback", 0.5).min(0.0).max(1.0))
		.dryLevel(newSynth.addParameter("delay_drylevel", 0.5).min(0.0).max(1.0))
		//.wetLevel(0.2);
		.wetLevel(newSynth.addParameter("delay_wetlevel", 0.5).min(0.0).max(1.0));

	Generator tone = delay.input(output * 0.25);
	//2023feb21, spi, end

	ADSR env = ADSR()
		.attack(0.04)
		.decay(0.1)
		.sustain(0.8)
		.release(0.6)
		.doesSustain(true)
		.trigger(gate);

	ControlGenerator filterFreq = voiceFreq * 0.5 + 200;

	LPF24 filter = LPF24().Q(1.0 + noteVelocity * 0.02).cutoff(filterFreq);

	//Generator output2 = ((tone * env) >> filter) * (0.02 + noteVelocity * 0.005);
	Generator output2 = ((tone * env)) * (0.02 + noteVelocity * 0.005);

	newSynth.setOutputGen(output2);

	return newSynth;
}




///////////////////////////
//based on compressor synth
///////////////////////////
Synth createSynthVoice_v6()
{
	Synth newSynth;

	ControlParameter noteNum = newSynth.addParameter("polyNote", 0.0);
	ControlParameter gate = newSynth.addParameter("polyGate", 0.0);
	ControlParameter noteVelocity = newSynth.addParameter("polyVelocity", 0.0);
	ControlParameter voiceNumber = newSynth.addParameter("polyVoiceNumber", 0.0);

	ControlGenerator voiceFreq = ControlMidiToFreq().input(noteNum) + voiceNumber * 1.2; // detune the voices slightly

	//2023feb21, spi, begin
	//Generator tone_final = SquareWave().freq(voiceFreq) * SineWave().freq(50);
	//ControlGenerator bpm = newSynth.addParameter("tempo", 120).min(1).max(300);
	ControlGenerator bpm = newSynth.addParameter("tempo", 240).min(1).max(300); //spi

	//ControlParameter threshold = addParameter("threshold", -12.f).displayName("Threshold (dbFS)").min(-60.f).max(0.f); //original
	ControlParameter threshold = newSynth.addParameter("comp_threshold", -12.f).displayName("Threshold (dbFS)").min(-60.f).max(0.f); //spi
	//ControlParameter ratio = addParameter("ratio", 2.0f).displayName("Ratio").min(1.0f).max(64.f).logarithmic(true); //original
	ControlParameter ratio = newSynth.addParameter("comp_ratio", 1.0f).displayName("Ratio").min(1.0f).max(64.f).logarithmic(true); //spi
	//ControlParameter attack = addParameter("attackTime", 0.001).displayName("Attack Time (s)").min(0.001).max(0.1).logarithmic(true); //original
	ControlParameter attack = newSynth.addParameter("comp_attack", 0.001).displayName("Attack Time (s)").min(0.001).max(0.1).logarithmic(true); //spi
	//ControlParameter release = addParameter("releaseTime", 0.05).displayName("Release Time (s)").min(0.01).max(0.08).logarithmic(true); //original
	ControlParameter release = newSynth.addParameter("comp_release", 0.06).displayName("Release Time (s)").min(0.01).max(0.08).logarithmic(true); //spi
	//ControlParameter gain = addParameter("gain", 0.f).displayName("Makeup Gain (dbFS)").min(0.f).max(36.0f); //original
	ControlParameter gain = newSynth.addParameter("comp_gain", 36.f).displayName("Makeup Gain (dbFS)").min(0.f).max(36.0f); //spi
	ControlParameter bypass = newSynth.addParameter("comp_bypass", false).parameterType(ControlParameterTypeToggle); //original
	//ControlParameter bypass = addParameter("bypass", true).parameterType(ControlParameterTypeToggle); //spi

	// Emulation of 808 snare with long, quiet noise decay
	//Generator hpNoise = (Noise() * dBToLin(-18.0)) >> HPF24().cutoff(2000.0) >> LPF12().cutoff(8000); //original
	//Generator hpNoise = (Noise() * dBToLin(-2.0)) >> HPF24().cutoff(10.0) >> LPF12().cutoff(2000); //spi
	Generator hpNoise = (Noise() * newSynth.addParameter("noise_amp", 1.0).min(0.0).max(1.0)) >> LPF12().cutoff(newSynth.addParameter("noise_cutoff", 2000.0).min(50.0).max(10000.0)); //spi
	//Generator tones = SineWave().freq(180) * dBToLin(-6.0) + SineWave().freq(332) * dBToLin(-18.0); //original
	Generator tones = SineWave().freq(voiceFreq) * newSynth.addParameter("tone_amp", 1.0).min(0.0).max(1.0); //spi

	//ControlGenerator snareGate = ControlPulse(60.0*0.8/80.0).trigger( ControlMetro(80) ); //original
	//ControlGenerator kickGate = ControlPulse(0.100).trigger(ControlMetro(120)); //spi
	//ControlGenerator kickGate = ControlPulse(0.100).trigger(ControlMetro(120)); //spi
	ControlGenerator kickGate = ControlPulse(0.100).trigger(ControlMetro().bpm(bpm)); //spi
	//ADSR toneADSR = ADSR(0.0005, 0.03, 0.0, 0.01).trigger(snareGate); //original
	//ADSR toneADSR = ADSR(0.0005, 1.25, 0.05, 0.1).trigger(kickGate); //spi

	ControlParameter tone_attack = newSynth.addParameter("tone_attack", 0.0005).min(0.0).max(5.0);
	ControlParameter tone_decay = newSynth.addParameter("tone_decay", 0.100).min(0.0).max(5.0);
	ControlParameter tone_sustain = newSynth.addParameter("tone_sustain", 0.010).min(0.0).max(5.0);
	ControlParameter tone_release = newSynth.addParameter("tone_release", 0.010).min(0.0).max(5.0);
	//ADSR toneADSR = ADSR(tone_attack.getValue(), tone_decay.getValue(), tone_sustain.getValue(), tone_release.getValue()).doesSustain(false).trigger(kickGate); //spi
	ADSR toneADSR = ADSR().attack(tone_attack).decay(tone_decay).sustain(tone_sustain).release(tone_release).doesSustain(false).trigger(kickGate); //spi
	//ADSR toneADSR = ADSR(0.0005, 0.100, 0.010, 0.010).doesSustain(false).trigger(kickGate); //spi
	//ADSR noiseADSR = ADSR(0.001, 0.25, 0.0, 0.25).trigger(kickGate); //original

	ControlParameter noise_attack = newSynth.addParameter("noise_attack", 0.001).min(0.0).max(5.0);
	ControlParameter noise_decay = newSynth.addParameter("noise_decay", 0.030).min(0.0).max(5.0);
	ControlParameter noise_sustain = newSynth.addParameter("noise_sustain", 0.000).min(0.0).max(5.0);
	ControlParameter noise_release = newSynth.addParameter("noise_release", 0.010).min(0.0).max(5.0);
	//ADSR noiseADSR = ADSR(noise_attack.getValue(), noise_decay.getValue(), noise_sustain.getValue(), noise_release.getValue()).doesSustain(false).trigger(kickGate); //spi
	//ADSR noiseADSR = ADSR(0.001, 0.030, 0.000, 0.010).doesSustain(false).trigger(kickGate); //spi
	ADSR noiseADSR = ADSR().attack(noise_attack).decay(noise_decay).sustain(noise_sustain).release(noise_release).doesSustain(false).trigger(kickGate); //spi

	// Squaring gives something a bit closer to exponential
	// TODO: Exponential ADSR
	Generator noiseEnv = noiseADSR * noiseADSR;

	Compressor compressor = Compressor()
		.attack(attack)
		.release(release)
		.threshold(ControlDbToLinear().input(threshold))
		.ratio(ratio)
		.lookahead(0.001)
		.makeupGain(ControlDbToLinear().input(gain))
		.bypass(bypass);

	Generator tone_final = (((tones * toneADSR) + (hpNoise * noiseEnv)) >> compressor) * 0.5;
	//2023feb21, spi, end

	ADSR env = ADSR()
		.attack(0.04)
		.decay(0.1)
		.sustain(0.8)
		.release(0.6)
		.doesSustain(true)
		.trigger(gate);

	ControlGenerator filterFreq = voiceFreq * 0.5 + 200;

	LPF24 filter = LPF24().Q(1.0 + noteVelocity * 0.02).cutoff(filterFreq);

	//Generator output = ((tone_final * env) >> filter) * (0.02 + noteVelocity * 0.005);
	Generator output = ((tone_final * env)) * (0.02 + noteVelocity * 0.005);

	newSynth.setOutputGen(output);

	return newSynth;
}




///////////////////////////
//based on the control snap to scale synth
///////////////////////////
Synth createSynthVoice_v7()
{
	Synth newSynth;

	ControlParameter noteNum = newSynth.addParameter("polyNote", 0.0);
	ControlParameter gate = newSynth.addParameter("polyGate", 0.0);
	ControlParameter noteVelocity = newSynth.addParameter("polyVelocity", 0.0);
	ControlParameter voiceNumber = newSynth.addParameter("polyVoiceNumber", 0.0);

	ControlGenerator voiceFreq = ControlMidiToFreq().input(noteNum) + voiceNumber * 1.2; // detune the voices slightly

	//2023feb21, spi, begin
	//Generator tone = SquareWave().freq(voiceFreq) * SineWave().freq(50);
	float scaleArr[6] = { 0, 2, 3, 7, 10 };
	vector<float> scale(scaleArr, scaleArr + sizeof scaleArr / sizeof scaleArr[0]);

	ControlGenerator speed = newSynth.addParameter("speed", 0.85); //original
	//ControlGenerator speed = addParameter("speed", 2.2); //spi

	ControlMetro switchIt = ControlMetro().bpm(30 * speed);

	/*
	ControlMetro metro = ControlMetro().bpm(
	ControlStepper()
	.bidirectional(1)
	.start(550)
	.end(650)
	.step(100)
	.trigger(switchIt)
	* speed
	);
	*/
	ControlMetro metro = ControlMetro().bpm(
		ControlStepper()
		.bidirectional(1)
		.start(150)
		.end(250)
		.step(100)
		.trigger(switchIt)
		* speed
		); //spi
	/*
	ADSR env = ADSR(0.01, 0.5, 0, 0)
	.trigger(metro)
	.doesSustain(false)
	.legato(true)
	.decay(
	ControlStepper()
	.start(0.05)
	.end(0.5)
	.step(0.01)
	.trigger(metro)
	);
	*/
	ADSR env = ADSR(0.01, 0.5, 0.5, 0)  //0.5 sustain
		.trigger(metro)
		.doesSustain(true)
		.legato(true)
		.decay(
		ControlStepper()
		.start(0.05)
		.end(0.5)
		.step(0.01)
		.trigger(metro)
		); //spi

	ControlGenerator stepperStart = newSynth.addParameter("stepperStart") * 30 + 43; //original
	//ControlGenerator stepperStart = addParameter("stepperStart", 0.0f) * 30 + 13; //spi

	ControlStepper stepper1 = ControlStepper()
		.bidirectional(1)
		.start(stepperStart)
		//.end(stepperStart + 50 * addParameter("stepperSpread")) //original
		.end(stepperStart + 50 * newSynth.addParameter("stepperSpread", 1.0f)) //spi
		.step(
		ControlStepper()
		.bidirectional(1)
		.start(4)
		.end(
		ControlRandom()
		.min(9)
		.max(15)
		.trigger(
		ControlMetro().bpm(11) //original
		//ControlMetro().bpm(50) //spi
		)
		)
		.step(1)
		.trigger(metro)
		)
		.trigger(metro);

	ControlSnapToScale scaleSnapper1 = ControlSnapToScale()
		.setScale(scale)
		.input(stepper1);

	//Generator delayMix = ControlRandom().min(0).max(0.1).trigger(switchIt).smoothed(); //original
	Generator delayMix = ControlRandom().min(0).max(1.0).trigger(switchIt).smoothed(); //spi

	//Reverb verb = Reverb().inputHPFCutoff(200.f).decayTime(0.8).roomSize(0.1).density(0.8).wetLevel(dBToLin(-30.f)); //original
	//Reverb verb = Reverb().inputHPFCutoff(200.f).decayTime(0.8).roomSize(0.1).density(0.8).wetLevel(dBToLin(-20.0f)); //spi
	//Reverb verb = Reverb().inputHPFCutoff(100.f).decayTime(0.8).roomSize(0.1).density(0.8).wetLevel(dBToLin(-20.0f)); //spi
	Reverb verb = Reverb().inputHPFCutoff(100.f).decayTime(0.8).roomSize(1.0).density(0.8).wetLevel(dBToLin(-10.0f)); //spi

	Generator outputGen = ((

		SineWave().freq( //original
		//SawtoothWave().freq( //spi
		ControlMidiToFreq().input(scaleSnapper1 + -12).smoothed().length(
		//ControlRandom().min(0).max(0.03).trigger(switchIt) //original
		ControlRandom().min(0.01).max(0.05).trigger(switchIt) //spi
		)
		)

		+

		/*
		SineWave().freq(
		ControlMidiToFreq().input(scaleSnapper1 + 12)
		//) * 0.1 //original
		) * 0.5 //spi
		*/
		SineWave().freq(voiceFreq) * 0.5 //spi

		)

		//* 0.3 * env) >> StereoDelay(0.3, 0.33).feedback(0.3).dryLevel(1.0 - delayMix).wetLevel(delayMix) >> verb; //original
		*0.3 * env) >> StereoDelay(1.3, 0.83).feedback(0.3).dryLevel(1.0 - delayMix).wetLevel(delayMix) >> verb; //spi
	//*0.3 * env) >> verb; //spi
	//*0.3 * env); //spi

	Generator tone = outputGen;
	//2023feb21, spi, end

	ADSR env2 = ADSR()
		.attack(0.04)
		.decay(0.1)
		.sustain(0.8)
		.release(0.6)
		.doesSustain(true)
		.trigger(gate);

	ControlGenerator filterFreq = voiceFreq * 0.5 + 200;

	LPF24 filter = LPF24().Q(1.0 + noteVelocity * 0.02).cutoff(filterFreq);

	//Generator output = ((tone * env2) >> filter) * (0.02 + noteVelocity * 0.005);
	Generator output = ((tone * env2)) * (0.02 + noteVelocity * 0.005);

	newSynth.setOutputGen(output);

	return newSynth;
}



//////////////////////////
//based on the delay synth
//////////////////////////
Synth createSynthVoice_v8()
{
	Synth newSynth;

	ControlParameter noteNum = newSynth.addParameter("polyNote", 0.0);
	ControlParameter gate = newSynth.addParameter("polyGate", 0.0);
	ControlParameter noteVelocity = newSynth.addParameter("polyVelocity", 0.0);
	ControlParameter voiceNumber = newSynth.addParameter("polyVoiceNumber", 0.0);

	ControlGenerator voiceFreq = ControlMidiToFreq().input(noteNum) + voiceNumber * 1.2; // detune the voices slightly

	//2023feb22, spi, begin
	//Generator tone = SquareWave().freq(voiceFreq) * SineWave().freq(50);
	//ControlParameter tempo = addParameter("tempo", 120.f).displayName("Tempo").min(60.f).max(300.f);
	ControlParameter tempo = newSynth.addParameter("tempo", 120.f).displayName("Tempo").min(20.f).max(300.f);
	//ControlParameter delayTime = addParameter("delayTime", 0.12f).displayName("Delay Time").min(0.001f).max(1.0f).logarithmic(true); // logarithmic
	ControlParameter delayTime = newSynth.addParameter("delayTime", 1.0f).displayName("Delay Time").min(0.001f).max(1.0f).logarithmic(true); // logarithmic
	ControlParameter feedBack = newSynth.addParameter("feedback", 0.4f).displayName("Delay Feedback").min(0.0f).max(0.95f);
	//ControlParameter delayMix = addParameter("delayMix", 0.3f).displayName("Delay Dry/Wet").min(0.0f).max(1.0f);
	ControlParameter delayMix = newSynth.addParameter("delayMix", 1.0f).displayName("Delay Dry/Wet").min(0.0f).max(1.0f);
	//ControlParameter decay = addParameter("decayTime", 0.08f).displayName("Env Decay Time").min(0.05f).max(0.25f).logarithmic(true); // logarithmic
	ControlParameter decay = newSynth.addParameter("decayTime", 0.5f).displayName("Env Decay Time").min(0.05f).max(0.5f).logarithmic(true); // logarithmic
	//ControlParameter volume = addParameter("volume", -6.f).displayName("Volume (dbFS)").min(-60.0f).max(0.f);
	ControlParameter volume = newSynth.addParameter("volume", 0.f).displayName("Volume (dbFS)").min(-60.0f).max(0.f);

	ControlMetro metro = ControlMetro().bpm(tempo * 4);

	ADSR aEnv = ADSR().attack(0.005f).decay(decay).sustain(0.0f).release(0.01f).trigger(metro).doesSustain(false).exponential(true);
	ADSR fEnv = ADSR().attack(0.005f).decay(decay).sustain(0.0f).release(0.01f).trigger(metro).doesSustain(false).exponential(true);

	float scalenums[5] = { 0, 3, 5, 7, 10 };
	std::vector<float> scale(scalenums, scalenums + 5);

	//ControlRandom rand = ControlRandom().min(0).max(36).trigger(metro); //original
	ControlRandom rand = ControlRandom().min(0).max(newSynth.addParameter("rand_max", 36.f).min(1.0f).max(72.f)).trigger(metro); //spi
	ControlSnapToScale snap = ControlSnapToScale().setScale(scale).input(rand);
	//ControlMidiToFreq freq = ControlMidiToFreq().input(48 + snap); //original
	ControlMidiToFreq freq = ControlMidiToFreq().input(newSynth.addParameter("base_freq", 36.f).min(1.0f).max(72.f) + snap); //spi

	ControlParameter pulsewidth = newSynth.addParameter("pulsewidth", 0.1f).min(0.050f).max(1.0f); //spi

	Generator osc = (
		(
		(
		//RectWave().freq(freq * 0.99).pwm(0.5f) * aEnv * 0.5
		RectWave().freq(voiceFreq * 0.99).pwm(pulsewidth) * aEnv * 0.5
		)
		>> MonoToStereoPanner().pan(-0.5)
		)
		+
		(
		(
		//RectWave().freq(freq * 1.01).pwm(0.5f) * aEnv * 0.5
		RectWave().freq(voiceFreq * 1.01).pwm(pulsewidth) * aEnv * 0.5
		)
		>> MonoToStereoPanner().pan(0.5)
		)
		); //original

	/*
	Generator osc = (
	(
	(
	SawtoothWave().freq(freq * 0.99) * aEnv * 0.5
	)
	>> MonoToStereoPanner().pan(-0.5)
	)
	+
	(
	(
	SawtoothWave().freq(freq * 1.01) * aEnv * 0.5
	)
	>> MonoToStereoPanner().pan(0.5)
	)
	);
	*/ //spi

	ControlParameter lpf_cutoff = newSynth.addParameter("lpf_cutoff", 400.0f).min(50).max(10000); //spi
	ControlParameter lpf_q = newSynth.addParameter("lpf_q", 1.1f).min(0.1).max(100); //spi

	//LPF12 filt = LPF12().cutoff(400.0f * (1.0f + fEnv*9.0f)).Q(1.1f);
	LPF12 filt = LPF12().cutoff(lpf_cutoff * (1.0f + fEnv*9.0f)).Q(lpf_q);

	Generator smoothMix = delayMix.smoothed();

	BasicDelay delay = BasicDelay(0.5f, 1.0f)
		.delayTime(delayTime.smoothed(0.5f))
		.feedback(feedBack.smoothed())
		.dryLevel(1.0f - smoothMix)
		.wetLevel(smoothMix);

	//Generator tone = (osc >> filt >> delay) * ControlDbToLinear().input(volume).smoothed();
	Generator tone = (osc >> filt) * ControlDbToLinear().input(volume).smoothed();
	//2023feb22, spi, end

	ADSR env = ADSR()
		.attack(0.04)
		.decay(0.1)
		.sustain(0.8)
		.release(0.6)
		.doesSustain(true)
		.trigger(gate);

	ControlGenerator filterFreq = voiceFreq * 0.5 + 200;

	LPF24 filter = LPF24().Q(1.0 + noteVelocity * 0.02).cutoff(filterFreq);

	//Generator output = ((tone * env) >> filter) * (0.02 + noteVelocity * 0.005);
	Generator output = ((tone * env)) * (0.02 + noteVelocity * 0.005);

	newSynth.setOutputGen(output);

	return newSynth;
}



/////////////////////////////////////
//based on the events synth
/////////////////////////////////////
Synth createSynthVoice_v9()
{
	Synth newSynth;

	ControlParameter noteNum = newSynth.addParameter("polyNote", 0.0);
	ControlParameter gate = newSynth.addParameter("polyGate", 0.0);
	ControlParameter noteVelocity = newSynth.addParameter("polyVelocity", 0.0);
	ControlParameter voiceNumber = newSynth.addParameter("polyVoiceNumber", 0.0);

	ControlGenerator voiceFreq = ControlMidiToFreq().input(noteNum) + voiceNumber * 1.2; // detune the voices slightly

	//2023feb22, spi, begin
	//Generator tone = SquareWave().freq(voiceFreq) * SineWave().freq(50);
	const int NUM_VOICES = 5;
	Generator allVoices;

	Generator noise = PinkNoise(); //original
	//Generator noise = 0.5* SineWave().freq(110) + 0.5* SineWave().freq(220); //spi
	//Generator noise = SineWave().freq(220); //spi
	//Generator noise = SawtoothWave().freq(220); //spi
	//Generator noise = LFNoise().setFreq(220); //spi

	vector<float> scale;
	scale.push_back(0);
	scale.push_back(2);
	scale.push_back(4);
	scale.push_back(7);
	scale.push_back(11);

	for (int i = 0; i < NUM_VOICES; i++)
	{
		////////////////////////////////////////////
		// Create the synthesis network
		////////////////////////////////////////////

		Generator voice;

		// Send a trigger message immediately, don't wait for the first beat to happen with the metrenome 
		ControlTrigger initialTrigger;
		initialTrigger.trigger();
		//ControlGenerator resetTrigger = initialTrigger + ControlMetro().bpm(ofRandom(10, 15)); //original - does not compile
		ControlGenerator resetTrigger = initialTrigger + ControlMetro().bpm(ControlRandom().min(10).max(15));

		ControlGenerator noiseTrigger = ControlMetro().bpm(ControlRandom().min(50).max(200).trigger(resetTrigger));

		ControlGenerator pulseLen = ControlRandom().min(0.1).max(0.5).trigger(resetTrigger);

		//ControlGenerator pulse = ControlPulse().length(pulseLen).input(noiseTrigger); //original - does not compile
		ControlGenerator pulse = ControlPulse().length(pulseLen).trigger(noiseTrigger);

		Generator env = ADSR(0.01, 0, 0.5, 0.01).decay(pulseLen * 0.5).trigger(pulse); //original
		//Generator env = ADSR(0.01, 0, 0.05, 0.01).decay(pulseLen * 0.5).trigger(pulse);

		voice = noise * env; //original
		//voice = noise ;

		ControlGenerator highPassAmount = ControlRandom().min(0).max(1).trigger(resetTrigger);

		ControlSnapToScale scaleSnapper = ControlSnapToScale().setScale(scale);
		scaleSnapper.input(40 + highPassAmount * 50); //original
		//scaleSnapper.input(10 + highPassAmount * 50); //spi
		ControlGenerator filterFreq = ControlMidiToFreq().input(scaleSnapper); //original
		//ControlGenerator filterFreq = ControlMidiToFreq().input(40 + highPassAmount * 50); //spi, bypass snap to scale
		ControlGenerator q = ControlRandom().min(0.5).max(1).trigger(resetTrigger);
		q = q * q * 50;

		//voice = HPF24().cutoff(filterFreq).input(voice).Q(q); //original
		//voice = LPF24().cutoff(filterFreq).input(voice).Q(q); //spi
		//voice = BPF24().cutoff(voiceFreq).input(voice).Q(q); //spi
		//voice = BPF24().cutoff(voiceFreq).input(voice).Q(q) + LPF24().cutoff(filterFreq).input(voice).Q(q); //spi 2023
		voice = BPF24().cutoff(voiceFreq).input(voice).Q(q) + 0.5* LPF24().cutoff(filterFreq).input(voice).Q(q); //spi 2023

		float pan = (2 * (float)i / NUM_VOICES) - 1;
		voice = MonoToStereoPanner().pan(pan).input(voice) * (15.0 / NUM_VOICES);

		allVoices = allVoices + voice;
	}

	//Generator allvoicesPlusReverb = Reverb().input(allVoices).density(1).dryLevel(0).wetLevel(0.2).decayTime(1); //original
	//Generator allvoicesPlusReverb = Reverb().input(allVoices).density(1).dryLevel(0).wetLevel(0.9).decayTime(1); //spi
	Generator allvoicesPlusReverb = Reverb().input(allVoices).density(1).dryLevel(0.1).wetLevel(0.9).decayTime(1); //spi

	Generator tone = allVoices + allvoicesPlusReverb;

	ADSR env_final = ADSR()
		.attack(0.04)
		.decay(0.1)
		.sustain(0.8)
		.release(0.6)
		.doesSustain(true)
		.trigger(gate);

	ControlGenerator filterFreq = voiceFreq * 0.5 + 200;

	LPF24 filter = LPF24().Q(1.0 + noteVelocity * 0.02).cutoff(filterFreq);

	//Generator output = ((tone * env_final) >> filter) * (0.02 + noteVelocity * 0.005);
	Generator output = ((tone * env_final)) * (0.02 + noteVelocity * 0.005);

	newSynth.setOutputGen(output);

	return newSynth;
}



/////////////////////////////////////
//based on the filtered noise synth
/////////////////////////////////////
Synth createSynthVoice_v10()
{
	Synth newSynth;

	ControlParameter noteNum = newSynth.addParameter("polyNote", 0.0);
	ControlParameter gate = newSynth.addParameter("polyGate", 0.0);
	ControlParameter noteVelocity = newSynth.addParameter("polyVelocity", 0.0);
	ControlParameter voiceNumber = newSynth.addParameter("polyVoiceNumber", 0.0);

	ControlGenerator voiceFreq = ControlMidiToFreq().input(noteNum) + voiceNumber * 1.2; // detune the voices slightly
	//2023feb22, spi, begin
	//Generator tone = SquareWave().freq(voiceFreq) * SineWave().freq(50);
	vector<float> midiNums;
	midiNums.push_back(-24);
	midiNums.push_back(0);
	midiNums.push_back(3);
	midiNums.push_back(7);
	midiNums.push_back(10);

	const float cutoffMult = 100;

	PinkNoise noise = PinkNoise();

	ControlGenerator cutoffCtrl = newSynth.addParameter("cutoff", 0.5);

	Generator q_v = newSynth.addParameter("Q", 5).smoothed();

	ControlGenerator lowBasFreq = ControlFloor().input(midiNums.at(0) + 12 + cutoffCtrl * cutoffMult) >> ControlMidiToFreq();
	ControlGenerator fmAmt = ControlValue(0.1);
	Generator cutoffSlowSwell = (SineWave().freq(0.1) + 1.0f) * 500;
	cutoffSlowSwell = FixedValue(500);
	Generator lpfCutoff = 100 + (cutoffSlowSwell + 0.5 * SineWave().freq(10) * cutoffSlowSwell);
	LPF12 filter = LPF12().cutoff(lpfCutoff);

	ControlGenerator toothyBassRandomAmp = ControlRandom()
		.min(-0.1)
		.max(1)
		.trigger(
		ControlMetro().bpm(ControlRandom().min(10).max(30))
		);

	Generator toothyBassSwell = ((toothyBassRandomAmp * toothyBassRandomAmp * toothyBassRandomAmp * 5) >> ControlPrinter().message("toothyBassSwell %f")).smoothed(10);

	Generator lowToothyBass =
		RectWave()
		.freq(lowBasFreq)
		.pwm(0.5 + 0.2 * (SineWave().freq(0.013) + 1));

	lowToothyBass = filter.input(lowToothyBass);
	lowToothyBass = lowToothyBass * toothyBassSwell;

	Adder sumOfFilters;

	for (int i = 0; i < midiNums.size(); i++)
	{
		Generator tremelo = (SineWave().freq(randomFloat(0.1, 0.3)) + 1.5) * 0.3;
		Generator cutoff = ControlMidiToFreq().input(ControlFloor().input(midiNums.at(i) + cutoffCtrl * cutoffMult)).smoothed().length(0.01);
		//BPF24 filter = BPF24().Q(q_v).cutoff(cutoff).normalizesGain(true);
		BPF24 filter = BPF24().Q(q_v).cutoff(voiceFreq).normalizesGain(true); //spi 2023
		sumOfFilters.input((noise >> filter) * tremelo);
	}

	// add a bit of gain for higher Q
	// Using this to test output limiter as well - this will probably clip/wrap if limiter is not working
	//Generator tone = sumOfFilters * (1 + q_v * 0.05) + lowToothyBass * 0.05;
	Generator tone = sumOfFilters * (1 + q_v * 0.5) + lowToothyBass * 0.5;
	//2023feb22, spi, end

	ADSR env = ADSR()
		.attack(0.04)
		.decay(0.1)
		.sustain(0.8)
		.release(0.6)
		.doesSustain(true)
		.trigger(gate);

	ControlGenerator filterFreq = voiceFreq * 0.5 + 200;

	LPF24 filter2 = LPF24().Q(1.0 + noteVelocity * 0.02).cutoff(filterFreq);

	//Generator output = ((tone * env) >> filter2) * (0.02 + noteVelocity * 0.005);
	Generator output = ((tone * env)) * (0.02 + noteVelocity * 0.005);

	newSynth.setOutputGen(output);

	return newSynth;
}


/////////////////////////////////////
//based on the filter synth
//note: it's a band pass filtered sample (buffer player sample) 
/////////////////////////////////////
Synth createSynthVoice_v11()
{
	Synth newSynth;

	ControlParameter noteNum = newSynth.addParameter("polyNote", 0.0);
	ControlParameter gate = newSynth.addParameter("polyGate", 0.0);
	ControlParameter noteVelocity = newSynth.addParameter("polyVelocity", 0.0);
	ControlParameter voiceNumber = newSynth.addParameter("polyVoiceNumber", 0.0);

	ControlGenerator voiceFreq = ControlMidiToFreq().input(noteNum) + voiceNumber * 1.2; // detune the voices slightly
	
	//2023feb22, spi, begin
	//Generator tone = SquareWave().freq(voiceFreq) * SineWave().freq(50);
	const int NUM_STEPS = 8;

	//SampleTable buffer = loadAudioFile("D:\\oifii-org\\httpdocs\\ha-org\\had\\dj-oifii\\worldaudio_wav\\00min15sec-and-less\\Geoffrey Oryema - TAO -  mara(introlater)_9sec.wav",2);
	SndfileHandle file1;
	//file1 = SndfileHandle("D:\\oifii-org\\httpdocs\\ha-org\\had\\dj-oifii\\worldaudio_wav\\00min15sec-and-less\\Geoffrey Oryema - TAO -  mara(introlater)_9sec.wav");
	file1 = SndfileHandle("D:\\oifii-org\\httpdocs\\ha-org\\had\\dj-oifii\\worldaudio_wav\\00min30sec-and-less\\GF - Subramanian - track 03(intro)_18sec.wav");
	assert(file1.samplerate() == 44100);
	assert(file1.channels() == 2);
	SampleTable buffer1(file1.frames(), file1.channels());
	file1.read(buffer1.dataPointer(), file1.frames()*file1.channels());
	BufferPlayer bPlayer1;
	bPlayer1.setBuffer(buffer1).loop(false);

	float file1duration_s = ((float)file1.frames()) / ((float)file1.samplerate());
	float file1duration_bpm = 60.f / file1duration_s;
	// synth paramters are like instance variables -- they're values you can set later, by calling synth.setParameter()
	ControlGenerator bpm = newSynth.addParameter("tempo", file1duration_bpm).min(file1duration_bpm).max(10.f*file1duration_bpm);
	//ControlGenerator bpm = addParameter("tempo", 80).min(50).max(300);

	// Send a trigger message immediately, don't wait for the first beat to happen with the metrenome 
	ControlTrigger initialTrigger;
	initialTrigger.trigger();

	// ControlMetro generates a "trigger" message at a given bpm. 
	ControlGenerator metro = initialTrigger + ControlMetro().bpm(bpm);


	ControlGenerator track1volume1 = newSynth.addParameter("track1volume1", 1.0f).min(0.0f).max(1.0f);
	Generator track1 = bPlayer1.trigger(metro) * track1volume1;
	BPF24 bandpassfilter = BPF24().cutoff(voiceFreq);
	track1 = track1 >> bandpassfilter;

	StereoDelay stereodelay = StereoDelay(3.0f, 3.0f)
		.delayTimeLeft(0.5 + SineWave().freq(0.2) * 0.01)
		.delayTimeRight(0.55 + SineWave().freq(0.23) * 0.01)
		//.feedback(0.3)
		.feedback(0.7)
		.dryLevel(0.2)
		//.dryLevel(0.8)
		//.wetLevel(0.2);
		.wetLevel(0.8);

	StereoDelay stereodelay2 = StereoDelay(3.0f, 3.0f)
		.delayTimeLeft(0.5 + SineWave().freq(0.2) * 0.01)
		.delayTimeRight(0.55 + SineWave().freq(0.23) * 0.01)
		//.feedback(0.3)
		.feedback(0.5 + SineWave().freq(1) * 0.3)
		.dryLevel(0.2)
		//.dryLevel(0.8)
		//.wetLevel(0.2);
		.wetLevel(0.8);

	HPF24 hpfilter = HPF24()
		.cutoff(5000 + SineWave().freq(0.1) * 5000)
		//.cutoff(6000 + SineWave().freq(1.0) * 5000)
		.Q(0.1);

	/*
	ControlParameter dry = addParameter("dry",-6.f).displayName("Dry Level (dbFS)").min(-60.f).max(0.f);
	ControlParameter wet = addParameter("wet",-20.f).displayName("Wet Level (dbFS)").min( -60.f).max(0.f);
	ControlParameter time = addParameter("decayTime", 1.0f).displayName("Decay Time (s)").min(0.1f).max(10.f);
	ControlParameter lowDecay = addParameter("lowDecay", 16000.0f).displayName("Decay Lowpass Cutoff (Hz)").min(4000.0f).max(20000.0f);
	ControlParameter hiDecay = addParameter("hiDecay",20.0f).displayName("Decay Highpass Cutoff (Hz)").min(20.f).max(250.f);
	ControlParameter preDelay = addParameter("preDelay", 0.001f).displayName("Pre-delay").min(0.001f).max(0.05f);
	ControlParameter inputLPF = addParameter("inputLPF",18000.f).displayName("Input LPF cutoff (Hz)").min(4000.0f).max(20000.0f);
	ControlParameter inputHPF = addParameter("inputHPF",20.f).displayName("Input HPF cutoff (Hz)").min(20.f).max(250.f);
	ControlParameter density = addParameter("density",0.5f).displayName("Density");
	ControlParameter shape = addParameter("shape",0.5f).displayName("Shape");
	ControlParameter size = addParameter("size",0.5f).displayName("Room Size");
	ControlParameter stereo = addParameter("stereo",0.5f).displayName("Stereo Width");
	*/
	Reverb reverb = Reverb()
		.preDelayTime(0.001f)
		.inputLPFCutoff(18000.f)
		.inputHPFCutoff(20.f)
		.decayTime(1.f)
		.decayLPFCutoff(16000.0f)
		.decayHPFCutoff(20.0f)
		.stereoWidth(0.5f)
		.density(0.5f)
		.roomShape(0.5f)
		.roomSize(0.5f)
		.dryLevel(ControlDbToLinear().input(-6.f))
		.wetLevel(ControlDbToLinear().input(-20.f));

	Reverb reverb2 = Reverb()
		.preDelayTime(0.001f)
		.inputLPFCutoff(18000.f)
		.inputHPFCutoff(20.f)
		.decayTime(1.f)
		.decayLPFCutoff(16000.0f)
		.decayHPFCutoff(20.0f)
		.stereoWidth(0.5f)
		.density(0.5f)
		.roomShape(0.5f)
		.roomSize(0.5f)
		.dryLevel(0.2) //.dryLevel(ControlDbToLinear().input(-20.f))
		.wetLevel(0.2);//.wetLevel(ControlDbToLinear().input(-6.f));

	LPF24 lpfilter = LPF24()
		.cutoff(5000 + SineWave().freq(1.0) * 5000)
		//.cutoff(6000 + SineWave().freq(1.0) * 5000)
		.Q(0.1);

	//Generator tone = hpfilter.input(track1) + reverb2.input(track1)*randomFloat(0.1, 0.5) + stereodelay2.input(track1)*randomFloat(0.1, 0.5);
	Generator tone = track1 + reverb2.input(track1)*randomFloat(0.1, 0.5) + stereodelay2.input(track1)*randomFloat(0.1, 0.5);
	//2023feb22, spi, end

	ADSR env = ADSR()
		.attack(0.04)
		.decay(0.1)
		.sustain(0.8)
		.release(0.6)
		.doesSustain(true)
		.trigger(gate);

	ControlGenerator filterFreq = voiceFreq * 0.5 + 200;

	LPF24 filter = LPF24().Q(1.0 + noteVelocity * 0.02).cutoff(filterFreq);

	//Generator output = ((tone * env) >> filter) * (0.02 + noteVelocity * 0.005);
	Generator output = ((tone * env)) * (0.02 + noteVelocity * 0.005);

	newSynth.setOutputGen(output);

	return newSynth;
}


/////////////////////////////////////
//based on the fm drone synth
/////////////////////////////////////
Synth createSynthVoice_v12()
{
	Synth newSynth;

	ControlParameter noteNum = newSynth.addParameter("polyNote", 0.0);
	ControlParameter gate = newSynth.addParameter("polyGate", 0.0);
	ControlParameter noteVelocity = newSynth.addParameter("polyVelocity", 0.0);
	ControlParameter voiceNumber = newSynth.addParameter("polyVoiceNumber", 0.0);

	ControlGenerator voiceFreq = ControlMidiToFreq().input(noteNum) + voiceNumber * 1.2; // detune the voices slightly

	//2023feb22, spi, begin
	//Generator tone = SquareWave().freq(voiceFreq) * SineWave().freq(50);
	//ControlParameter volume = addParameter("volume", -12.f).displayName("Volume (dbFS)").min(-60.f).max(0.f); //original
	ControlParameter volume = newSynth.addParameter("volume", -0.0f).displayName("Volume (dbFS)").min(-60.f).max(0.f); //spi
	//ControlParameter carrierPitch = addParameter("carrierPitch", 28.f).displayName("Carrier Pitch").min(20.f).max(32.f); //original
	ControlParameter carrierPitch = newSynth.addParameter("carrierPitch", 32.f).displayName("Carrier Pitch").min(20.f).max(64.f); //spi
	//ControlParameter modIndex = addParameter("modIndex", 0.25f).displayName("FM Amount").min(0.f).max(1.0f); //original
	ControlParameter modIndex = newSynth.addParameter("modIndex", 1.0f).displayName("FM Amount").min(0.f).max(1.0f); //spi
	//ControlParameter lfoAmt = addParameter("lfoAmt", 0.5f).displayName("LFO Amount").min(0.f).max(1.f); //original
	ControlParameter lfoAmt = newSynth.addParameter("lfoAmt", 1.0f).displayName("LFO Amount").min(0.f).max(1.f); //spi

	//Generator rCarrierFreq = ControlMidiToFreq().input(carrierPitch).smoothed();
	Generator rCarrierFreq = ControlMidiToFreq().input(noteNum).smoothed();
	Generator rModFreq = rCarrierFreq * 4.0f;

	Generator tone = SineWave()
		.freq(rCarrierFreq
		+ (
		SineWave().freq(rModFreq) *
		rModFreq *
		(modIndex.smoothed() * (1.0f + SineWave().freq((LFNoise().setFreq(0.5f) + 1.f) * 2.f + 0.2f) * (lfoAmt * 0.5f).smoothed()))
		)
		) * ControlDbToLinear().input(volume).smoothed() * ((SineWave().freq(0.15f) + 1.f) * 0.75f + 0.25);
	//2023feb22, spi, end

	ADSR env = ADSR()
		.attack(0.04)
		.decay(0.1)
		.sustain(0.8)
		.release(0.6)
		.doesSustain(true)
		.trigger(gate);

	ControlGenerator filterFreq = voiceFreq * 0.5 + 200;

	LPF24 filter = LPF24().Q(1.0 + noteVelocity * 0.02).cutoff(filterFreq);

	//Generator output = ((tone * env) >> filter) * (0.02 + noteVelocity * 0.005);
	Generator output = ((tone * env)) * (0.02 + noteVelocity * 0.005);

	newSynth.setOutputGen(output);

	return newSynth;
}



/////////////////////////////////////
//based on the lf noise synth
/////////////////////////////////////
Synth createSynthVoice_v13()
{
	Synth newSynth;

	ControlParameter noteNum = newSynth.addParameter("polyNote", 0.0);
	ControlParameter gate = newSynth.addParameter("polyGate", 0.0);
	ControlParameter noteVelocity = newSynth.addParameter("polyVelocity", 0.0);
	ControlParameter voiceNumber = newSynth.addParameter("polyVoiceNumber", 0.0);

	ControlGenerator voiceFreq = ControlMidiToFreq().input(noteNum) + voiceNumber * 1.2; // detune the voices slightly

	//2023feb22, spi, begin
	//Generator tone = SquareWave().freq(voiceFreq) * SineWave().freq(50);
	ControlParameter pitch = newSynth.addParameter("sinefreq", 500).min(10.0).max(10000.0); //spi

	Generator tone = SineWave().freq(
		//500 + 500 * LFNoise().setFreq( //original
		//pitch + pitch * LFNoise().setFreq(
		voiceFreq + voiceFreq *LFNoise().setFreq(
		//addParameter("noiseFreq", 100)) //original
		newSynth.addParameter("noisefreq", 100).min(1.0).max(10000.0)) //spi
		)
		//* 0.3; //original
		*newSynth.addParameter("vol", 1.0).min(0.0).max(1.0); //spi
	//2023feb22, spi, end

	ADSR env = ADSR()
		.attack(0.04)
		.decay(0.1)
		.sustain(0.8)
		.release(0.6)
		.doesSustain(true)
		.trigger(gate);

	ControlGenerator filterFreq = voiceFreq * 0.5 + 200;

	LPF24 filter = LPF24().Q(1.0 + noteVelocity * 0.02).cutoff(filterFreq);

	//Generator output = ((tone * env) >> filter) * (0.02 + noteVelocity * 0.005);
	Generator output = ((tone * env)) * (0.02 + noteVelocity * 0.005);

	newSynth.setOutputGen(output);

	return newSynth;
}



/////////////////////////////////////
//based on the reverb synth
/////////////////////////////////////
Synth createSynthVoice_v14()
{
	Synth newSynth;

	ControlParameter noteNum = newSynth.addParameter("polyNote", 0.0);
	ControlParameter gate = newSynth.addParameter("polyGate", 0.0);
	ControlParameter noteVelocity = newSynth.addParameter("polyVelocity", 0.0);
	ControlParameter voiceNumber = newSynth.addParameter("polyVoiceNumber", 0.0);

	ControlGenerator voiceFreq = ControlMidiToFreq().input(noteNum) + voiceNumber * 1.2; // detune the voices slightly

	//2023feb22, spi, begin
	//Generator tone = SquareWave().freq(voiceFreq) * SineWave().freq(50);
	ControlParameter dry = newSynth.addParameter("dry", -6.f).displayName("Dry Level (dbFS)").min(-60.f).max(0.f);
	ControlParameter wet = newSynth.addParameter("wet", -20.f).displayName("Wet Level (dbFS)").min(-60.f).max(0.f);
	ControlParameter time = newSynth.addParameter("decayTime", 1.0f).displayName("Decay Time (s)").min(0.1f).max(10.f);
	ControlParameter lowDecay = newSynth.addParameter("lowDecay", 16000.0f).displayName("Decay Lowpass Cutoff (Hz)").min(4000.0f).max(20000.0f);
	ControlParameter hiDecay = newSynth.addParameter("hiDecay", 20.0f).displayName("Decay Highpass Cutoff (Hz)").min(20.f).max(250.f);
	ControlParameter preDelay = newSynth.addParameter("preDelay", 0.001f).displayName("Pre-delay").min(0.001f).max(0.05f);
	ControlParameter inputLPF = newSynth.addParameter("inputLPF", 18000.f).displayName("Input LPF cutoff (Hz)").min(4000.0f).max(20000.0f);
	ControlParameter inputHPF = newSynth.addParameter("inputHPF", 20.f).displayName("Input HPF cutoff (Hz)").min(20.f).max(250.f);
	ControlParameter density = newSynth.addParameter("density", 0.5f).displayName("Density");
	ControlParameter shape = newSynth.addParameter("shape", 0.5f).displayName("Shape");
	ControlParameter size = newSynth.addParameter("size", 0.5f).displayName("Room Size");
	ControlParameter stereo = newSynth.addParameter("stereo", 0.5f).displayName("Stereo Width");

	//float bpm = 120.f;
	//float bpm = 180.f;
	float bpm = 360.f;

	ControlMetro beat = ControlMetro().bpm(bpm);
	ControlDelay offbeat = ControlDelay(1.2f).input(beat).delayTime(30.0f / bpm);

	Generator click = Noise() * ADSR(0.0001f, 0.025f, 0, 0.01f).doesSustain(false).exponential(true).trigger(beat);

	//Generator tone = RectWave().pwm(0.5f).freq(Tonic::mtof(60)) * ADSR(0.001f, 0.08f, 0, 0.01f).doesSustain(false).exponential(true).trigger(offbeat);
	//Generator tone = RectWave().pwm(0.5f).freq(voiceFreq) * ADSR(0.001f, 0.08f, 0, 0.01f).doesSustain(false).exponential(true).trigger(offbeat);
	Generator tone = RectWave().freq(voiceFreq) * ADSR(0.001f, 0.08f, 0, 0.01f).doesSustain(false).exponential(true).trigger(offbeat);

	Reverb reverb = Reverb()
		.preDelayTime(preDelay)
		.inputLPFCutoff(inputLPF)
		.inputHPFCutoff(inputHPF)
		.decayTime(time)
		.decayLPFCutoff(lowDecay)
		.decayHPFCutoff(hiDecay)
		.stereoWidth(stereo)
		.density(density)
		.roomShape(shape)
		.roomSize(size)
		.dryLevel(ControlDbToLinear().input(dry))
		.wetLevel(ControlDbToLinear().input(wet));

	//Generator tone2 = ((click + tone) >> reverb) * 0.8f; //works with the click too!
	//Generator tone2 = ((tone) >> reverb) * 0.8f;
	Generator tone2 = ((tone) >> reverb) * 2.0f;
	//2023feb22, spi, end

	ADSR env = ADSR()
		.attack(0.04)
		.decay(0.1)
		.sustain(0.8)
		.release(0.6)
		.doesSustain(true)
		.trigger(gate);

	ControlGenerator filterFreq = voiceFreq * 0.5 + 200;

	LPF24 filter = LPF24().Q(1.0 + noteVelocity * 0.02).cutoff(filterFreq);

	//Generator output = ((tone * env) >> filter) * (0.02 + noteVelocity * 0.005);
	Generator output = ((tone2 * env)) * (0.02 + noteVelocity * 0.005);

	newSynth.setOutputGen(output);

	return newSynth;
}




/////////////////////////////////////
//based on the simple step seq synth
/////////////////////////////////////
Synth createSynthVoice_v15()
{
	Synth newSynth;

	ControlParameter noteNum = newSynth.addParameter("polyNote", 0.0);
	ControlParameter gate = newSynth.addParameter("polyGate", 0.0);
	ControlParameter noteVelocity = newSynth.addParameter("polyVelocity", 0.0);
	ControlParameter voiceNumber = newSynth.addParameter("polyVoiceNumber", 0.0);

	ControlGenerator voiceFreq = ControlMidiToFreq().input(noteNum) + voiceNumber * 1.2; // detune the voices slightly

	//2023feb23, spi, begin
	//Generator tone = SquareWave().freq(voiceFreq) * SineWave().freq(50);
	const int NUM_STEPS = 8; //original
	//const int NUM_STEPS = 64; //spi

	ControlGenerator metro = ControlMetro().bpm(4 * newSynth.addParameter("tempo", 100).min(50).max(300));

	ControlGenerator step = ControlStepper().end(NUM_STEPS).trigger(metro);
	ControlGenerator transpose = newSynth.addParameter("transpose", 0).min(-6).max(6);
	ControlSwitcher pitches = ControlSwitcher().inputIndex(step);
	ControlSwitcher cutoffs = ControlSwitcher().inputIndex(step);

	for (int i = 0; i < NUM_STEPS; i++)
	{
		//float initialValue = randomFloat(10, 80);
		float minimalValue = 10; //noteNum.getValue() ;
		float maximalValue = 80; //noteNum.getValue() + 12;
		float initialValue = randomFloat(minimalValue, maximalValue);
		pitches.addInput(newSynth.addParameter("step" + to_string(i) + "Pitch", initialValue).min(minimalValue).max(maximalValue));
		cutoffs.addInput(newSynth.addParameter("step" + to_string(i) + "Cutoff", 500).min(30).max(1500));
	}

	//ControlGenerator midiNote = transpose + (pitches >> ControlSnapToScale().setScale({0,2,3,5,7,10})); //original
	ControlGenerator midiNote = transpose + pitches;
	Generator tone = SquareWave().freq(midiNote >> ControlMidiToFreq()) * ADSR(0.01, 0.1, 0, 0).trigger(metro); // *0.1;
	//2023feb23, spi, end
	ADSR env = ADSR()
		.attack(0.04)
		.decay(0.1)
		.sustain(0.8)
		.release(0.6)
		.doesSustain(true)
		.trigger(gate);

	ControlGenerator filterFreq = voiceFreq * 0.5 + 200;

	LPF24 filter = LPF24().Q(1.0 + noteVelocity * 0.02).cutoff(filterFreq);

	//Generator output = ((tone * env) >> filter) * (0.02 + noteVelocity * 0.005);
	Generator output = ((tone * env)) * (0.02 + noteVelocity * 0.005);

	newSynth.setOutputGen(output);

	return newSynth;
}



/////////////////////////////////////
//based on the sine sum synth
/////////////////////////////////////
Synth createSynthVoice_v16()
{
	Synth newSynth;

	ControlParameter noteNum = newSynth.addParameter("polyNote", 0.0);
	ControlParameter gate = newSynth.addParameter("polyGate", 0.0);
	ControlParameter noteVelocity = newSynth.addParameter("polyVelocity", 0.0);
	ControlParameter voiceNumber = newSynth.addParameter("polyVoiceNumber", 0.0);

	ControlGenerator voiceFreq = ControlMidiToFreq().input(noteNum) + voiceNumber * 1.2; // detune the voices slightly

	//2023feb23, spi, begin
	//Generator tone = SquareWave().freq(voiceFreq) * SineWave().freq(50);
	const int NUM_SINES=10;

	//ControlParameter pitch = addParameter("pitch",0); //original
	ControlParameter pitchmulti = newSynth.addParameter("pitchmulti", 0.0).min(0.0).max(10.0); //spi
	ControlParameter pitchbase = newSynth.addParameter("pitchbase", 110.0).min(20.0).max(10000.0); //spi

	Adder outputAdder;

	for (int s = 0; s<NUM_SINES; s++)
	{
		// ControlGenerator pitchGen = ((pitch * 220 + 220) * powf(2, (s - (NUM_SINES/2)) * 5.0f / 12.0f)); //original
		ControlGenerator pitchGen = ((pitchmulti * voiceFreq + voiceFreq) * powf(2, (s - (NUM_SINES / 2)) * 5.0f / 12.0f));
		outputAdder.input(SineWave().freq(pitchGen.smoothed()));
	}
	//Generator tone = outputAdder * ((1.0f / NUM_SINES) * 0.5f);
	//Generator tone = outputAdder * ((1.0f / NUM_SINES) );
	Generator tone = outputAdder * ((1.0f / NUM_SINES) * 2.0f);
	//2023feb23, spi, end
	
	ADSR env = ADSR()
		.attack(0.04)
		.decay(0.1)
		.sustain(0.8)
		.release(0.6)
		.doesSustain(true)
		.trigger(gate);

	ControlGenerator filterFreq = voiceFreq * 0.5 + 200;

	LPF24 filter = LPF24().Q(1.0 + noteVelocity * 0.02).cutoff(filterFreq);

	//Generator output = ((tone * env) >> filter) * (0.02 + noteVelocity * 0.005);
	Generator output = ((tone * env)) * (0.02 + noteVelocity * 0.005);
	//Generator output = ((tone * env)); // *(0.02 + noteVelocity * 0.005);

	newSynth.setOutputGen(output);

	return newSynth;
}

