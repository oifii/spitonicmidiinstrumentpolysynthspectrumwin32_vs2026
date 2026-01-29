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


////////////////////////////////////////////////////////////////
//nakedsoftware.org, spi@oifii.org or stephane.poirier@oifii.org
//
//2015dec08, creation of spitonicmidiinstrumentsynthwin32.cpp 
//
//2015dec08, showbytes() has not been revised, replace putchar()
//
//nakedsoftware.org, spi@oifii.org or stephane.poirier@oifii.org
////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "spitonicmidiinstrumentsynthwin32.h"
#include "FreeImage.h"
#include <shellapi.h> //for CommandLineToArgW()
#include <mmsystem.h> //for timeSetEvent()
#include <stdio.h> //for swprintf()
#include <assert.h>
#include "spiwavsetlib.h"

#include "porttime.h"
#include "portmidi.h"
#include <map>

#include "portaudio.h"
#include "pa_asio.h"


#include "Tonic.h"

#include "ControlSwitcherTestSynth.h"
#include "ControlSwitcherExpSynth.h"
#include "BasicSynth.h"
#include "SimpleInstrumentSynth.h"
#include "SimpleInstrumentBufferPlayerSynth.h"
#include "SimpleInstrumentTableLookupSynth.h"
#include "SimpleInstrumentTableLookupSPEARSynth.h"
#include "SimpleInstrumentSineSumSynth.h"
#include "SimpleInstrumentBasicSynth.h"
#include "StepSequencerSynth.h"
#include "StepSequencerExpSynth.h"
#include "StepSequencerBufferPlayerExpSynth.h"
#include "StepSequencerBufferPlayerEffectExpSynth.h"
//#include "EventsSynth.h"
#include "EventsExpSynth.h"
#include "BufferPlayerExpSynth.h"
#include "ArbitraryTableLookupSynth.h"
#include "BandlimitedOscillatorTestSynth.h"
#include "CompressorDuckingTestSynth.h"
#include "CompressorTestSynth.h"
#include "CompressorExpSynth.h"
#include "ControlSnapToScaleTestSynth.h"
#include "DelayTestSynth.h"
#include "FilteredNoiseSynth.h"
#include "FilterExpSynth.h"
#include "FMDroneSynth.h"
#include "InputDemoSynth.h"
#include "LFNoiseTestSynth.h"
#include "ReverbTestSynth.h"
#include "SimpleStepSeqSynth.h"
#include "SineSumSynth.h"
#include "StereoDelayTestSynth.h"
#include "SynthsAsGeneratorsDemoSynth.h"
#include "XYSpeedSynth.h"
#include "PolySynth.h"

#include "spitonicsynths.h"

//2024mar10, spi, begin
//fourier.h is the addon from audio programming book to the rfftw library,
//see audio programming book page 536 for details. in short, fourier.cpp
//wraps the rfftw by providing 2 functions: fft() and ifft().
//fourier.h also depends on libsndfile so it makes rfftw.lib depends on
//libsndfile for compiling (no need for linking it if you don't use it
//elsewhere).
//fft() function can only be called always with the same sample size N,
//this because within fft() implementation rfftw_create_plan() is called
//only once (the first time fft() is called).
#include <fourier.h> //in rfftw.lib (static library)
#include "spiutility.h"
//2024mar10, spi, end

#include "defs.h"
#include "spiaudiodevice.h"
int global_numchannels = NUM_CHANNELS;
SPIAudioDevice mySPIAudioDevice;

using namespace Tonic;

/*
#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (1024) //#define FRAMES_PER_BUFFER (512) //#define FRAMES_PER_BUFFER (2048) //#define FRAMES_PER_BUFFER (64) 
//#define NUM_CHANNELS    (1)
#define NUM_CHANNELS    (2)
*/

// Static smart pointer for our Synth
/*
static Synth synth;
*/
//static ControlSwitcherTestSynth synth;
//static ControlSwitcherExpSynth synth;
//static BasicSynth synth;
//static SimpleInstrumentSynth synth;
//static SimpleInstrumentBufferPlayerSynth synth;
//static SimpleInstrumentTableLookupSynth synth;
//static SimpleInstrumentTableLookupSPEARSynth synth;
static PolySynth poly;
static Synth synth;
//static SimpleInstrumentSineSumSynth synth;
//static SimpleInstrumentBasicSynth synth;
//static StepSequencerSynth synth;
//static StepSequencerExpSynth synth;
//static StepSequencerBufferPlayerExpSynth synth;
//static StepSequencerBufferPlayerEffectExpSynth synth;
//static EventsSynth synth;
//static EventsExpSynth synth;
//static BufferPlayerExpSynth synth;
//static ArbitraryTableLookupSynth synth;
//static BandlimitedOscillatorTestSynth synth;
//static CompressorDuckingTestSynth synth;
//static CompressorTestSynth synth;
//static CompressorExpSynth synth;
//static ControlSnapToScaleTestSynth synth;
//static DelayTestSynth synth;
//static FilteredNoiseSynth synth;
//static FilterExpSynth synth;
//static FMDroneSynth synth;
//static InputDemoSynth synth;
//static LFNoiseTestSynth synth;
//static ReverbTestSynth synth;
//static SimpleStepSeqSynth synth;
//static SineSumSynth synth;
//static StereoDelayTestSynth synth;
//static SynthsAsGeneratorsDemoSynth synth;
//static XYSpeedSynth synth;


/*
// Select sample format. 
#if 1
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"
#elif 1
#define PA_SAMPLE_TYPE  paInt16
typedef short SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#elif 0
#define PA_SAMPLE_TYPE  paInt8
typedef char SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#else
#define PA_SAMPLE_TYPE  paUInt8
typedef unsigned char SAMPLE;
#define SAMPLE_SILENCE  (128)
#define PRINTF_S_FORMAT "%d"
#endif
*/

// Global Variables:

CHAR pCHAR[1024];
WCHAR pWCHAR[1024];

PmStream* global_pPmStreamMIDIIN;      // midi input 
bool global_active = false;     // set when global_pPmStreamMIDIIN is ready for reading
bool global_inited = false;     // suppress printing during command line parsing 
int global_inputmidideviceid =  11; //alesis q49 midi port id (when midi yoke installed)
std::map<string,int> global_inputmididevicemap;

//string global_instrumentnamepattern="";
string global_inputmididevicename = "Q49"; //"In From MIDI Yoke:  1", "In From MIDI Yoke:  2", ... , "In From MIDI Yoke:  8"
int global_inputmidichannel=0;
//string global_audiodevicename="E-MU ASIO"; //"Speakers (2- E-MU E-DSP Audio Processor (WDM))"
//string global_audiodevicename="Speakers (2- E-MU E-DSP Audio P"; //"E-MU ASIO"
//int global_outputAudioChannelSelectors[2]; 
std::map<string,int> global_devicemap;

//Instrument* global_pInstrument=NULL;

#define MAX_LOADSTRING 100
FIBITMAP* global_dib;
HFONT global_hFont;
HWND global_hwnd=NULL;
MMRESULT global_timer=0;
#define MAX_GLOBALTEXT	4096
WCHAR global_text[MAX_GLOBALTEXT+1];
int global_x=100;
int global_y=200;
int global_xwidth=400;
int global_yheight=400;
BYTE global_alpha=200;
int global_fontheight=24;
int global_fontwidth=-1; //will be computed within WM_PAINT handler
BYTE global_fontcolor_r=255;
BYTE global_fontcolor_g=255;
BYTE global_fontcolor_b=255;
int global_staticalignment = 0; //0 for left, 1 for center and 2 for right
int global_staticheight=-1; //will be computed within WM_SIZE handler
int global_staticwidth=-1; //will be computed within WM_SIZE handler 
//spi, begin
int global_imageheight=-1; //will be computed within WM_SIZE handler
int global_imagewidth=-1; //will be computed within WM_SIZE handler 
//spi, end
int global_titlebardisplay=1; //0 for off, 1 for on
int global_acceleratoractive=1; //0 for off, 1 for on
int global_menubardisplay=0; //0 for off, 1 for on
FILE* global_pfile=NULL;
#define IDC_MAIN_EDIT	100
#define IDC_MAIN_STATIC	101

HINSTANCE hInst;								// current instance
//TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
//TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
TCHAR szTitle[1024]={L"spitonicmidiinstrumentsynthwin32title"};					// The title bar text
TCHAR szWindowClass[1024]={L"spitonicmidiinstrumentsynthwin32class"};			// the main window class name

//new parameters
string global_begin="begin.ahk";
string global_end="end.ahk";

//#define StatusAddText StatusAddTextW

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

//2024mar10, spi, begin
/*
// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring &wstr)
{
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo( size_needed, 0 );
    WideCharToMultiByte                  (CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo( size_needed, 0 );
    MultiByteToWideChar                  (CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}
*/
//2024mar10, spi, end

/*
// Select sample format
#if 1
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"
#elif 1
#define PA_SAMPLE_TYPE  paInt16
typedef short SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#elif 0
#define PA_SAMPLE_TYPE  paInt8
typedef char SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#else
#define PA_SAMPLE_TYPE  paUInt8
typedef unsigned char SAMPLE;
#define SAMPLE_SILENCE  (128)
#define PRINTF_S_FORMAT "%d"
#endif
*/

/* //now in spiaudiodevice.h
std::map<string, int> global_inputdevicemap;
std::map<string, int> global_outputdevicemap;
*/

/* //now in spiaudiodevice.h
PaStream* global_stream;
PaStreamParameters global_inputParameters;
PaStreamParameters global_outputParameters;
PaError global_err;
string global_audioinputdevicename = "";
string global_audiooutputdevicename = "";
int global_inputAudioChannelSelectors[2];
int global_outputAudioChannelSelectors[2];
PaAsioStreamInfo global_asioInputInfo;
PaAsioStreamInfo global_asioOutputInfo;
*/

FILE* pFILE = NULL;



#define MIDI_CODE_MASK  0xf0
#define MIDI_CHN_MASK   0x0f
//#define MIDI_REALTIME   0xf8
//  #define MIDI_CHAN_MODE  0xfa 
#define MIDI_OFF_NOTE   0x80
#define MIDI_ON_NOTE    0x90
#define MIDI_POLY_TOUCH 0xa0
#define MIDI_CTRL       0xb0
#define MIDI_CH_PROGRAM 0xc0
#define MIDI_TOUCH      0xd0
#define MIDI_BEND       0xe0

#define MIDI_SYSEX      0xf0
#define MIDI_Q_FRAME	0xf1
#define MIDI_SONG_POINTER 0xf2
#define MIDI_SONG_SELECT 0xf3
#define MIDI_TUNE_REQ	0xf6
#define MIDI_EOX        0xf7
#define MIDI_TIME_CLOCK 0xf8
#define MIDI_START      0xfa
#define MIDI_CONTINUE	0xfb
#define MIDI_STOP       0xfc
#define MIDI_ACTIVE_SENSING 0xfe
#define MIDI_SYS_RESET  0xff

#define MIDI_ALL_SOUND_OFF 0x78
#define MIDI_RESET_CONTROLLERS 0x79
#define MIDI_LOCAL	0x7a
#define MIDI_ALL_OFF	0x7b
#define MIDI_OMNI_OFF	0x7c
#define MIDI_OMNI_ON	0x7d
#define MIDI_MONO_ON	0x7e
#define MIDI_POLY_ON	0x7f

bool in_sysex = false;   // we are reading a sysex message 
bool done = false;       // when true, exit 
bool notes = true;       // show notes? 
bool controls = true;    // show continuous controllers 
bool bender = true;      // record pitch bend etc.? 
bool excldata = true;    // record system exclusive data? 
bool verbose = true;     // show text representation? 
bool realdata = true;    // record real time messages? 
bool clksencnt = true;   // clock and active sense count on 
bool chmode = true;      // show channel mode messages 
bool pgchanges = true;   // show program changes 
bool flush = false;	    // flush all pending MIDI data 

uint32_t filter = 0;            // remember state of midi filter 

uint32_t clockcount = 0;        // count of clocks 
uint32_t actsensecount = 0;     // cout of active sensing bytes 
uint32_t notescount = 0;        // #notes since last request 
uint32_t notestotal = 0;        // total #notes 

char val_format[] = "    Val %d\n";



//2024mar10, spi, begin
int global_soundnumber = 0;
int global_textdisplay = 1; //1 for enabled, 0 for disabled
int global_spectrumdisplay = 1; //1 for enabled, 0 for disabled
//2024mar10, spi, end

//2024mar10, spi, begin
int timer_delay_ms = 25; //update spectrum refresh rate
DWORD timer = 0;
HDC specdc = 0;
HBITMAP specbmp = 0;
BYTE* specbuf;
int specmode = 0, specpos = 0; // spectrum mode (and marker pos for 2nd mode)
int global_idcolorpalette = 0;
int global_bands = 20;
int SPECWIDTH = 500;	// display width
int SPECHEIGHT = 250;	// display height 
int prev_specmode = 0;
//#define SPECTRUMWINDOW_MAXAUDIOBUFFERSIZE	(2*1920*2)	//max 2 channels and 2 screens 1920x1080
//#define SPECTRUMWINDOW_MAXAUDIOBUFFERSIZE	(2*2048*2)	//max 2 channels * max framesperbuffer (should be greater than width of a 4K screen or greater than width of 2 screens of 1920x1080)
#define SPECTRUMWINDOW_MAXNUMCHANNELS		2
//#define SPECTRUMWINDOW_MAXFRAMEBUFFER		(2*2048)
//2 * FRAMES_PER_BUFFER
#define SPECTRUMWINDOW_MAXFRAMESPERBUFFER	(2*FRAMES_PER_BUFFER)
//2 * max num channels * max framesperbuffer * 2 (for 4K displays, has to be greater than width of a 4K screen or greater than width of 2 screens of 1920x1080)
#define SPECTRUMWINDOW_MAXAUDIOBUFFERSIZE	(2*SPECTRUMWINDOW_MAXNUMCHANNELS*SPECTRUMWINDOW_MAXFRAMESPERBUFFER)	

bool audiobuffer_ready = false;
float audiobuffer[SPECTRUMWINDOW_MAXAUDIOBUFFERSIZE];

float buf[SPECTRUMWINDOW_MAXAUDIOBUFFERSIZE];
float buf2[SPECTRUMWINDOW_MAXAUDIOBUFFERSIZE];
float fftbuf[SPECTRUMWINDOW_MAXAUDIOBUFFERSIZE];
//2024mar10, spi, end





bool global_abort = false;

static int renderCallback(const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData);

static int gNumNoInputs = 0;
// This routine will be called by the PortAudio engine when audio is needed.
// It may be called at interrupt level on some machines so don't do anything
// that could mess up the system like calling malloc() or free().
//
static int renderCallback(const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData)
{
	SAMPLE *out = (SAMPLE*)outputBuffer;
	const SAMPLE *in = (const SAMPLE*)inputBuffer;
	unsigned int i;
	(void)timeInfo; // Prevent unused variable warnings.
	(void)statusFlags;
	(void)userData;

	if (global_abort == true) return paAbort;

	/*
	if( inputBuffer == NULL )
	{
	for( i=0; i<framesPerBuffer; i++ )
	{
	*out++ = 0;  // left - silent
	*out++ = 0;  // right - silent
	}
	gNumNoInputs += 1;
	}
	else
	{
	for (i = 0; i<framesPerBuffer; i++)
	{
	*out++ = *in++;  // left - unprocessed
	*out++ = *in++;  // right - unprocessed
	}
	}
	*/


	//synth.fillBufferOfFloats((float*)outputBuffer, nBufferFrames, NUM_CHANNELS);
	synth.fillBufferOfFloats((float*)outputBuffer, framesPerBuffer, NUM_CHANNELS);
	//2024mar10, spi, begin
	audiobuffer_ready = false;
	memcpy(audiobuffer, outputBuffer, sizeof(float) * framesPerBuffer * NUM_CHANNELS);
	audiobuffer_ready = true;
	//2024mar10, spi, end
	return paContinue;
}

/*
bool SelectAudioInputDevice()
{
	const PaDeviceInfo* deviceInfo;
	int numDevices = Pa_GetDeviceCount();
	for (int i = 0; i<numDevices; i++)
	{
		deviceInfo = Pa_GetDeviceInfo(i);
		string devicenamestring = deviceInfo->name;
		global_inputdevicemap.insert(pair<string, int>(devicenamestring, i));
		if (pFILE) fprintf(pFILE, "id=%d, name=%s\n", i, devicenamestring.c_str());
	}

	int deviceid = Pa_GetDefaultInputDevice(); // default input device 
	std::map<string, int>::iterator it;
	it = global_inputdevicemap.find(global_audioinputdevicename);
	if (it != global_inputdevicemap.end())
	{
		deviceid = (*it).second;
		//printf("%s maps to %d\n", global_audiodevicename.c_str(), deviceid);
		deviceInfo = Pa_GetDeviceInfo(deviceid);
		//assert(inputAudioChannelSelectors[0]<deviceInfo->maxInputChannels);
		//assert(inputAudioChannelSelectors[1]<deviceInfo->maxInputChannels);
	}
	else
	{
		//Pa_Terminate();
		//return -1;
		//printf("error, audio device not found, will use default\n");
		//MessageBox(win,"error, audio device not found, will use default\n",0,0);
		deviceid = Pa_GetDefaultInputDevice();
	}


	global_inputParameters.device = deviceid;
	if (global_inputParameters.device == paNoDevice)
	{
		//MessageBox(win,"error, no default input device.\n",0,0);
		return false;
	}
	//global_inputParameters.channelCount = 2;
	global_inputParameters.channelCount = NUM_CHANNELS;
	global_inputParameters.sampleFormat = PA_SAMPLE_TYPE;
	global_inputParameters.suggestedLatency = Pa_GetDeviceInfo(global_inputParameters.device)->defaultLowOutputLatency;
	//inputParameters.hostApiSpecificStreamInfo = NULL;

	//Use an ASIO specific structure. WARNING - this is not portable. 
	//PaAsioStreamInfo asioInputInfo;
	global_asioInputInfo.size = sizeof(PaAsioStreamInfo);
	global_asioInputInfo.hostApiType = paASIO;
	global_asioInputInfo.version = 1;
	global_asioInputInfo.flags = paAsioUseChannelSelectors;
	global_asioInputInfo.channelSelectors = global_inputAudioChannelSelectors;
	if (deviceid == Pa_GetDefaultInputDevice())
	{
		global_inputParameters.hostApiSpecificStreamInfo = NULL;
	}
	else if (Pa_GetHostApiInfo(Pa_GetDeviceInfo(deviceid)->hostApi)->type == paASIO)
	{
		global_inputParameters.hostApiSpecificStreamInfo = &global_asioInputInfo;
	}
	else if (Pa_GetHostApiInfo(Pa_GetDeviceInfo(deviceid)->hostApi)->type == paWDMKS)
	{
		global_inputParameters.hostApiSpecificStreamInfo = NULL;
	}
	else
	{
		//assert(false);
		global_inputParameters.hostApiSpecificStreamInfo = NULL;
	}
	return true;
}



bool SelectAudioOutputDevice()
{
	const PaDeviceInfo* deviceInfo;
	int numDevices = Pa_GetDeviceCount();
	for (int i = 0; i<numDevices; i++)
	{
		deviceInfo = Pa_GetDeviceInfo(i);
		string devicenamestring = deviceInfo->name;
		global_outputdevicemap.insert(pair<string, int>(devicenamestring, i));
		if (pFILE) fprintf(pFILE, "id=%d, name=%s\n", i, devicenamestring.c_str());
	}

	int deviceid = Pa_GetDefaultOutputDevice(); // default output device 
	std::map<string, int>::iterator it;
	it = global_outputdevicemap.find(global_audiooutputdevicename);
	if (it != global_outputdevicemap.end())
	{
		deviceid = (*it).second;
		//printf("%s maps to %d\n", global_audiodevicename.c_str(), deviceid);
		deviceInfo = Pa_GetDeviceInfo(deviceid);
		//assert(inputAudioChannelSelectors[0]<deviceInfo->maxInputChannels);
		//assert(inputAudioChannelSelectors[1]<deviceInfo->maxInputChannels);
	}
	else
	{
		//Pa_Terminate();
		//return -1;
		//printf("error, audio device not found, will use default\n");
		//MessageBox(win,"error, audio device not found, will use default\n",0,0);
		deviceid = Pa_GetDefaultOutputDevice();
	}


	global_outputParameters.device = deviceid;
	if (global_outputParameters.device == paNoDevice)
	{
		//MessageBox(win,"error, no default output device.\n",0,0);
		return false;
	}
	//global_inputParameters.channelCount = 2;
	global_outputParameters.channelCount = NUM_CHANNELS;
	global_outputParameters.sampleFormat = PA_SAMPLE_TYPE;
	global_outputParameters.suggestedLatency = Pa_GetDeviceInfo(global_outputParameters.device)->defaultLowOutputLatency;
	//outputParameters.hostApiSpecificStreamInfo = NULL;

	//Use an ASIO specific structure. WARNING - this is not portable. 
	//PaAsioStreamInfo asioInputInfo;
	global_asioOutputInfo.size = sizeof(PaAsioStreamInfo);
	global_asioOutputInfo.hostApiType = paASIO;
	global_asioOutputInfo.version = 1;
	global_asioOutputInfo.flags = paAsioUseChannelSelectors;
	global_asioOutputInfo.channelSelectors = global_outputAudioChannelSelectors;
	if (deviceid == Pa_GetDefaultOutputDevice())
	{
		global_outputParameters.hostApiSpecificStreamInfo = NULL;
	}
	else if (Pa_GetHostApiInfo(Pa_GetDeviceInfo(deviceid)->hostApi)->type == paASIO)
	{
		global_outputParameters.hostApiSpecificStreamInfo = &global_asioOutputInfo;
	}
	else if (Pa_GetHostApiInfo(Pa_GetDeviceInfo(deviceid)->hostApi)->type == paWDMKS)
	{
		global_outputParameters.hostApiSpecificStreamInfo = NULL;
	}
	else
	{
		//assert(false);
		global_outputParameters.hostApiSpecificStreamInfo = NULL;
	}
	return true;
}
*/


/////////////////////////////////////////////////////////////////////////////
//               put_pitch
// Inputs:
//    int p: pitch number
// Effect: write out the pitch name for a given number
/////////////////////////////////////////////////////////////////////////////

static int put_pitch(int p)
{
    char result[8];
    static char *ptos[] = {
        "c", "cs", "d", "ef", "e", "f", "fs", "g",
        "gs", "a", "bf", "b"    };
    // note octave correction below 
    sprintf(result, "%s%d", ptos[p % 12], (p / 12) - 1);
    sprintf(pCHAR, "%s", result);StatusAddTextA(pCHAR);
    return strlen(result);
}


/////////////////////////////////////////////////////////////////////////////
//               showbytes
// Effect: print hex data, precede with newline if asked
/////////////////////////////////////////////////////////////////////////////

char nib_to_hex[] = "0123456789ABCDEF";

static void showbytes(PmMessage data, int len, bool newline)
{
    int count = 0;
    int i;

//    if (newline) {
//        putchar('\n');
//        count++;
//    } 
    for (i = 0; i < len; i++) 
	{
        putchar(nib_to_hex[(data >> 4) & 0xF]);
        putchar(nib_to_hex[data & 0xF]);
        count += 2;
        if (count > 72) 
		{
            putchar('.');
            putchar('.');
            putchar('.');
            break;
        }
        data >>= 8;
    }
    putchar(' ');
}

///////////////////////////////////////////////////////////////////////////////
//               output
// Inputs:
//    data: midi message buffer holding one command or 4 bytes of sysex msg
// Effect: format and print  midi data
///////////////////////////////////////////////////////////////////////////////
char vel_format[] = "    Vel %d\n";
static void output(PmMessage data)
{
    int command;    // the current command 
    int chan;   // the midi channel of the current event 
    int len;    // used to get constant field width 

    // printf("output data %8x; ", data); 

    command = Pm_MessageStatus(data) & MIDI_CODE_MASK;
    chan = Pm_MessageStatus(data) & MIDI_CHN_MASK;

    if (in_sysex || Pm_MessageStatus(data) == MIDI_SYSEX) {
#define sysex_max 16
        int i;
        PmMessage data_copy = data;
        in_sysex = true;
        // look for MIDI_EOX in first 3 bytes 
        // if realtime messages are embedded in sysex message, they will
        // be printed as if they are part of the sysex message
        //
        for (i = 0; (i < 4) && ((data_copy & 0xFF) != MIDI_EOX); i++) 
            data_copy >>= 8;
        if (i < 4) {
            in_sysex = false;
            i++; // include the EOX byte in output 
        }
        showbytes(data, i, verbose);
        if (verbose) 
		{
			sprintf(pCHAR, "System Exclusive\n");StatusAddTextA(pCHAR);
		}
    } else if (command == MIDI_ON_NOTE && Pm_MessageData2(data) != 0) {
        notescount++;
        if (notes) {
            showbytes(data, 3, verbose);
            if (verbose) 
			{
                sprintf(pCHAR, "NoteOn  Chan %2d Key %3d ", chan, Pm_MessageData1(data));StatusAddTextA(pCHAR);
                len = put_pitch(Pm_MessageData1(data));
                sprintf(pCHAR, vel_format + len, Pm_MessageData2(data));StatusAddTextA(pCHAR);
            }
        }
    } else if ((command == MIDI_ON_NOTE // && Pm_MessageData2(data) == 0
                || command == MIDI_OFF_NOTE) && notes) {
        showbytes(data, 3, verbose);
        if (verbose) 
		{
            sprintf(pCHAR, "NoteOff Chan %2d Key %3d ", chan, Pm_MessageData1(data));StatusAddTextA(pCHAR);
            len = put_pitch(Pm_MessageData1(data));
            sprintf(pCHAR, vel_format + len, Pm_MessageData2(data));StatusAddTextA(pCHAR);
        }
    } else if (command == MIDI_CH_PROGRAM && pgchanges) {
        showbytes(data, 2, verbose);
        if (verbose) 
		{
            sprintf(pCHAR, "  ProgChg Chan %2d Prog %2d\n", chan, Pm_MessageData1(data) + 1);StatusAddTextA(pCHAR);
        }
    } else if (command == MIDI_CTRL) {
               // controls 121 (MIDI_RESET_CONTROLLER) to 127 are channel
               // mode messages. 
        if (Pm_MessageData1(data) < MIDI_ALL_SOUND_OFF) {
            showbytes(data, 3, verbose);
            if (verbose) 
			{
                sprintf(pCHAR, "CtrlChg Chan %2d Ctrl %2d Val %2d\n",
                       chan, Pm_MessageData1(data), Pm_MessageData2(data));StatusAddTextA(pCHAR);
            }
        } else if (chmode) { // channel mode 
            showbytes(data, 3, verbose);
            if (verbose) {
                switch (Pm_MessageData1(data)) 
				{
                  case MIDI_ALL_SOUND_OFF:
                      sprintf(pCHAR, "All Sound Off, Chan %2d\n", chan);StatusAddTextA(pCHAR);
                    break;
                  case MIDI_RESET_CONTROLLERS:
                    sprintf(pCHAR, "Reset All Controllers, Chan %2d\n", chan);StatusAddTextA(pCHAR);
                    break;
                  case MIDI_LOCAL:
                    sprintf(pCHAR, "LocCtrl Chan %2d %s\n",
                            chan, Pm_MessageData2(data) ? "On" : "Off");StatusAddTextA(pCHAR);
                    break;
                  case MIDI_ALL_OFF:
                    sprintf(pCHAR, "All Off Chan %2d\n", chan);StatusAddTextA(pCHAR);
                    break;
                  case MIDI_OMNI_OFF:
                    sprintf(pCHAR, "OmniOff Chan %2d\n", chan);StatusAddTextA(pCHAR);
                    break;
                  case MIDI_OMNI_ON:
                    sprintf(pCHAR, "Omni On Chan %2d\n", chan);StatusAddTextA(pCHAR);
                    break;
                  case MIDI_MONO_ON:
                    sprintf(pCHAR, "Mono On Chan %2d\n", chan);StatusAddTextA(pCHAR);
                    if (Pm_MessageData2(data))
					{
                        sprintf(pCHAR, " to %d received channels\n", Pm_MessageData2(data));StatusAddTextA(pCHAR);
					}
                    else
					{
                        sprintf(pCHAR, " to all received channels\n");StatusAddTextA(pCHAR);
					}
                    break;
                  case MIDI_POLY_ON:
                    sprintf(pCHAR, "Poly On Chan %2d\n", chan);StatusAddTextA(pCHAR);
                    break;
                }
            }
        }
    } else if (command == MIDI_POLY_TOUCH && bender) {
        showbytes(data, 3, verbose);
        if (verbose) 
		{
            sprintf(pCHAR, "P.Touch Chan %2d Key %2d ", chan, Pm_MessageData1(data));StatusAddTextA(pCHAR);
            len = put_pitch(Pm_MessageData1(data));
            printf(val_format + len, Pm_MessageData2(data));
        }
    } else if (command == MIDI_TOUCH && bender) {
        showbytes(data, 2, verbose);
        if (verbose) 
		{
            sprintf(pCHAR, "  A.Touch Chan %2d Val %2d\n", chan, Pm_MessageData1(data));StatusAddTextA(pCHAR);
        }
    } else if (command == MIDI_BEND && bender) {
        showbytes(data, 3, verbose);
        if (verbose) 
		{
            sprintf(pCHAR, "P.Bend  Chan %2d Val %2d\n", chan,
                    (Pm_MessageData1(data) + (Pm_MessageData2(data)<<7)));StatusAddTextA(pCHAR);
        }
    } else if (Pm_MessageStatus(data) == MIDI_SONG_POINTER) {
        showbytes(data, 3, verbose);
        if (verbose) 
		{
            sprintf(pCHAR, "    Song Position %d\n",
                    (Pm_MessageData1(data) + (Pm_MessageData2(data)<<7)));StatusAddTextA(pCHAR);
        }
    } else if (Pm_MessageStatus(data) == MIDI_SONG_SELECT) {
        showbytes(data, 2, verbose);
        if (verbose) 
		{
            sprintf(pCHAR, "    Song Select %d\n", Pm_MessageData1(data));StatusAddTextA(pCHAR);
        }
    } else if (Pm_MessageStatus(data) == MIDI_TUNE_REQ) {
        showbytes(data, 1, verbose);
        if (verbose) 
		{
            sprintf(pCHAR, "    Tune Request\n");StatusAddTextA(pCHAR);
        }
    } else if (Pm_MessageStatus(data) == MIDI_Q_FRAME && realdata) {
        showbytes(data, 2, verbose);
        if (verbose) 
		{
            sprintf(pCHAR, "    Time Code Quarter Frame Type %d Values %d\n",
                    (Pm_MessageData1(data) & 0x70) >> 4, Pm_MessageData1(data) & 0xf);StatusAddTextA(pCHAR);
        }
    } else if (Pm_MessageStatus(data) == MIDI_START && realdata) {
        showbytes(data, 1, verbose);
        if (verbose) 
		{
            sprintf(pCHAR, "    Start\n");StatusAddTextA(pCHAR);
        }
    } else if (Pm_MessageStatus(data) == MIDI_CONTINUE && realdata) {
        showbytes(data, 1, verbose);
        if (verbose) 
		{
            sprintf(pCHAR, "    Continue\n");StatusAddTextA(pCHAR);
        }
    } else if (Pm_MessageStatus(data) == MIDI_STOP && realdata) {
        showbytes(data, 1, verbose);
        if (verbose) 
		{
            sprintf(pCHAR, "    Stop\n");StatusAddTextA(pCHAR);
        }
    } else if (Pm_MessageStatus(data) == MIDI_SYS_RESET && realdata) {
        showbytes(data, 1, verbose);
        if (verbose) 
		{
            sprintf(pCHAR, "    System Reset\n");StatusAddTextA(pCHAR);
        }
    } else if (Pm_MessageStatus(data) == MIDI_TIME_CLOCK) {
        if (clksencnt) clockcount++;
        else if (realdata) {
            showbytes(data, 1, verbose);
            if (verbose) 
			{
                sprintf(pCHAR, "    Clock\n");StatusAddTextA(pCHAR);
            }
        }
    } else if (Pm_MessageStatus(data) == MIDI_ACTIVE_SENSING) {
        if (clksencnt) actsensecount++;
        else if (realdata) {
            showbytes(data, 1, verbose);
            if (verbose) 
			{
                sprintf(pCHAR, "    Active Sensing\n");StatusAddTextA(pCHAR);
            }
        }
    } else showbytes(data, 3, verbose);
    fflush(stdout);
}

void receive_poll(PtTimestamp timestamp, void *userData)
{
    PmEvent event;
    int count; 
    if (!global_active) return;
    while ((count = Pm_Read(global_pPmStreamMIDIIN, &event, 1))) 
	{
        if (count == 1) 
		{
			//0) detect channel
			int command = Pm_MessageStatus(event.message) & MIDI_CODE_MASK;
			int chan = Pm_MessageStatus(event.message) & MIDI_CHN_MASK;
			int data1 = Pm_MessageData1(event.message);
			int data2 = Pm_MessageData2(event.message);
			if(chan==global_inputmidichannel)
			{
				//1) output message
				if(global_textdisplay) output(event.message);

				//2) 
				if (command == MIDI_OFF_NOTE || (command == MIDI_ON_NOTE && data2==0))
				{
					int midinotenumber = data1; //range 0 to 127
					int midinotevelocity = data2; //range 0 to 127
					poly.noteOff(midinotenumber);
				}
				else if (command == MIDI_ON_NOTE)
				{
					/*
					//2.1) set a parameter that we created when we defined the synth
					synth.setParameter("midiNote", midinotenumber);
					synth.setParameter("midiNoteVelocity", midinotevelocity);
					//2.2) trigger note
					//simply setting the value of a parameter causes that parameter to send a "trigger" message to any using them as triggers
					synth.setParameter("trigger", 1);
					*/
					int midinotenumber = data1; //range 0 to 127
					int midinotevelocity = data2; //range 0 to 127
					poly.noteOn(midinotenumber, midinotevelocity);
				}
				else if (command == MIDI_CH_PROGRAM)
				{

				}
				else if (command == MIDI_CTRL)
				{
					if(data1==0 && data2==0)
					{
						/*
						//output(event.message);
						ShellExecuteA(NULL, "open", ".\\begin.ahk", "", NULL, false);
						*/
					}
				}


			}

			

		}
        else            
		{
			//printf(Pm_GetErrorText((PmError)count)); //spi a cast as (PmError)
			sprintf(pCHAR, Pm_GetErrorText((PmError)count));StatusAddTextA(pCHAR);
		}
    }
}




//2024mar10, spi, begin
//2024feb21, spi, begin
int getv(int& x, int& c, int framesperbuffer, int numchannels) //int getv(int &x, int &c, int &framesperbuffer, int &numchannels)
{
	/*
	int v = (1 - _buf[x*numchannels + c])*_SPECHEIGHT / 2; // invert and scale to fit display //2021nov29, spi, was //int v = (1 - _buf[x*NUM_CHANNELS + c])*_SPECHEIGHT / 2; // invert and scale to fit display
	if (v<0) v = 0;
	else if (v >= _SPECHEIGHT) v = _SPECHEIGHT - 1;
	*/
	int v = 0;
	if (framesperbuffer < SPECWIDTH)
	{
		int x_inframes = (int)((float)x * ((float)framesperbuffer) / ((float)SPECWIDTH));
		v = (1 - buf[x_inframes * numchannels + c]) * SPECHEIGHT / 2;
	}
	else
	{
		v = (1 - buf[x * numchannels + c]) * SPECHEIGHT / 2;
	}
	if (v < 0) v = 0;
	else if (v >= SPECHEIGHT) v = SPECHEIGHT - 1;
	return v;
}
//2024feb21, spi, end

// update the spectrum display - the interesting bit :)
void CALLBACK UpdateSpectrum(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	HDC dc;
	int x, y, y1;

	//2024feb20, spi, begin
	//overriding user-specified global_bands
	if (1)
	{
		global_bands = SPECWIDTH / 16; //proportional, 120 bands for 2K display, 240 bands for 4K display, etc.
	}
	else
	{
		global_bands = 1920 / 16; //constant, 16 pixels per band, 120 bands on any display
	}
	if (global_bands < 4) global_bands = 4;
	if (global_bands > (FRAMES_PER_BUFFER - 2)) global_bands = (FRAMES_PER_BUFFER - 2);
	//2024feb20, spi, end

	if (specmode == 3 || specmode == 4 || specmode == 5 || specmode == 6 ||
		specmode == 7 || specmode == 8 || specmode == 9 || specmode == 10 ||
		specmode == 11 || specmode == 12 || specmode == 13 || specmode == 14 ||
		specmode == 15 || specmode == 16 || specmode == 17 || specmode == 18)
	{ // waveform and filled waveform
		int c;
		//2024feb20, spi, begin
		//float *buf;
		//2024feb20, spi, end
		//BASS_CHANNELINFO ci;

		//black background
		if (specmode == 3 || specmode == 4 || specmode == 5 || specmode == 6)
		{
			memset(specbuf, 0, SPECWIDTH * SPECHEIGHT);
		}
		//noisy background
		else if (specmode == 7 || specmode == 8 || specmode == 9 || specmode == 10)
		{
			for (int i = 0; i < SPECWIDTH; i++)
			{
				for (int j = 0; j < SPECHEIGHT; j++)
				{

					int random_integer;
					//int lowest=1, highest=127;
					int lowest = 64, highest = 96; //good
					int range = (highest - lowest) + 1;
					random_integer = lowest + int(range * rand() / (RAND_MAX + 1.0));
					specbuf[j * SPECWIDTH + i] = random_integer;
				}
			}
		}
		//solid slightly shifting background
		else if (specmode == 11 || specmode == 12 || specmode == 13 || specmode == 14)
		{
			int random_integer;
			//int lowest=1, highest=127;
			//int lowest=1, highest=255;
			int lowest = 64, highest = 96; //good
			int range = (highest - lowest) + 1;
			random_integer = lowest + int(range * rand() / (RAND_MAX + 1.0));
			memset(specbuf, random_integer, SPECWIDTH * SPECHEIGHT);
		}
		//solid radically shifting background
		else //specmode==15 || specmode==16 || specmode==17 || specmode==18
		{
			int random_integer;
			//int lowest=1, highest=127;
			int lowest = 1, highest = 255;
			//int lowest=64, highest=96; //good
			int range = (highest - lowest) + 1;
			random_integer = lowest + int(range * rand() / (RAND_MAX + 1.0));
			memset(specbuf, random_integer, SPECWIDTH * SPECHEIGHT);
		}

		//BASS_ChannelGetInfo(chan,&ci); // get number of channels
		//buf=(float*)alloca(ci.chans*SPECWIDTH*sizeof(float)); // allocate buffer for data
		//2024feb20, spi, begin
		//buf=(float*)alloca(NUM_CHANNELS*SPECWIDTH*sizeof(float)); // allocate buffer for data
		//2024feb20, spi, end
		//BASS_ChannelGetData(chan,buf,(ci.chans*SPECWIDTH*sizeof(float))|BASS_DATA_FLOAT); // get the sample data (floating-point to avoid 8 & 16 bit processing)

		//2024feb20, spi, begin
		/*
		//global_err = Pa_ReadStream( global_stream, buf, FRAMES_PER_BUFFER );
		mySPIAudioDevice.global_err = Pa_ReadStream(mySPIAudioDevice.global_stream, buf, NUM_CHANNELS*SPECWIDTH );
		if(mySPIAudioDevice.global_err != paNoError )
		{
			//char errorbuf[2048];
			//sprintf(errorbuf, "Error reading stream: %s\n", Pa_GetErrorText(global_err));
			//MessageBox(0,errorbuf,0,MB_ICONERROR);
			return;
		}
		*/
		while (!audiobuffer_ready) Sleep(1);
		memcpy(buf, audiobuffer, sizeof(float) * NUM_CHANNELS * FRAMES_PER_BUFFER); //we have
		//memcpy(buf, _audiobuffer, sizeof(float)*NUM_CHANNELS*SPECWIDTH); //we need
		//2024feb20, spi, end

		//under waveform filled down to bottom
		if (specmode == 6 || specmode == 10 || specmode == 14 || specmode == 18)
		{
			for (c = 0; c < NUM_CHANNELS; c++)
			{
				for (x = 0; x < SPECWIDTH; x++)
				{
					//2024feb21, spi, begin
					/*
					int v=(1-buf[x*NUM_CHANNELS+c])*SPECHEIGHT/2; // invert and scale to fit display
					if (v<0) v=0;
					else if (v>=SPECHEIGHT) v=SPECHEIGHT-1;
					*/
					// invert and scale to fit display
					int v = getv(x, c, FRAMES_PER_BUFFER, NUM_CHANNELS);
					//2024feb21, spi, end
					//under waveform filled down to bottom
					y = v;
					while (--y >= 0) specbuf[y * SPECWIDTH + x] = c & 1 ? 127 : 1;
				}
			}
		}
		//waveform filled towards center
		else if (specmode == 5 || specmode == 9 || specmode == 13 || specmode == 17)
		{
			for (c = 0; c < NUM_CHANNELS; c++)
			{
				for (x = 0; x < SPECWIDTH; x++)
				{
					//2024feb21, spi, begin
					/*
					int v=(1-buf[x*NUM_CHANNELS+c])*SPECHEIGHT/2; // invert and scale to fit display
					if (v<0) v=0;
					else if (v>=SPECHEIGHT) v=SPECHEIGHT-1;
					*/
					// invert and scale to fit display
					int v = getv(x, c, FRAMES_PER_BUFFER, NUM_CHANNELS);
					//2024feb21, spi, end
					//waveform filled towards center
					y = v;
					if (y > (SPECHEIGHT / 2))
						while (--y >= (SPECHEIGHT / 2)) specbuf[y * SPECWIDTH + x] = c & 1 ? 127 : 1;
					else if (y < (SPECHEIGHT / 2))
						while (++y <= (SPECHEIGHT / 2)) specbuf[y * SPECWIDTH + x] = c & 1 ? 127 : 1;
					else specbuf[y * SPECWIDTH + x] = c & 1 ? 127 : 1;
				}
			}
		}
		//waveform filled towards opposite
		else if (specmode == 4 || specmode == 8 || specmode == 12 || specmode == 16)
		{
			for (c = 0; c < NUM_CHANNELS; c++)
			{
				for (x = 0; x < SPECWIDTH; x++)
				{
					//2024feb21, spi, begin
					/*
					int v=(1-buf[x*NUM_CHANNELS+c])*SPECHEIGHT/2; // invert and scale to fit display
					if (v<0) v=0;
					else if (v>=SPECHEIGHT) v=SPECHEIGHT-1;
					*/
					// invert and scale to fit display
					int v = getv(x, c, FRAMES_PER_BUFFER, NUM_CHANNELS);
					//2024feb21, spi, end
					//waveform filled towards opposite
					y = v;
					if (y > (SPECHEIGHT / 2))
						while (--y >= (SPECHEIGHT / 2 - (v - (SPECHEIGHT / 2)))) specbuf[y * SPECWIDTH + x] = c & 1 ? 127 : 1;
					else if (y < (SPECHEIGHT / 2))
						while (++y <= (SPECHEIGHT / 2 + ((SPECHEIGHT / 2) - v))) specbuf[y * SPECWIDTH + x] = c & 1 ? 127 : 1;
					else specbuf[y * SPECWIDTH + x] = c & 1 ? 127 : 1;
				}
			}
		}
		//waveform (original)
		else if (specmode == 3 || specmode == 7 || specmode == 11 || specmode == 15)
		{
			for (c = 0; c < NUM_CHANNELS; c++)
			{
				for (x = 0; x < SPECWIDTH; x++)
				{
					//2024feb21, spi, begin
					/*
					int v=(1-buf[x*NUM_CHANNELS+c])*SPECHEIGHT/2; // invert and scale to fit display
					if (v<0) v=0;
					else if (v>=SPECHEIGHT) v=SPECHEIGHT-1;
					*/
					// invert and scale to fit display
					int v = getv(x, c, FRAMES_PER_BUFFER, NUM_CHANNELS);
					//2024feb21, spi, end
					if (!x) y = v;
					do
					{ // draw line from previous sample...
						if (y < v) y++;
						else if (y > v) y--;
						specbuf[y * SPECWIDTH + x] = c & 1 ? 127 : 1; // left=green, right=red (could add more colours to palette for more chans)
					} while (y != v);
				}
			}
		}
	}
	else
	{
		//2024feb20, spi, begin
		//float fftbuf[1024];
		//2024feb20, spi, end
		//BASS_ChannelGetData(chan,fft,BASS_DATA_FFT2048); // get the FFT data
		//2024feb20, spi, begin
		//float *buf2;
		//2024feb20, spi, end
		int numberofsamples = FRAMES_PER_BUFFER; // =SPECWIDTH;
		//if(numberofsamples<1024) numberofsamples=1024;
		//2024feb20, spi, begin
		//buf2=(float*)alloca(NUM_CHANNELS*numberofsamples*sizeof(float)); // allocate buffer for data
		/*
		mySPIAudioDevice.global_err = Pa_ReadStream(mySPIAudioDevice.global_stream, buf2, NUM_CHANNELS*numberofsamples );
		if(mySPIAudioDevice.global_err != paNoError )
		{
			//char errorbuf[2048];
			//sprintf(errorbuf, "Error reading stream (2): %s\n", Pa_GetErrorText(global_err));
			//MessageBox(0,errorbuf,0,MB_ICONERROR);
			return;
		}
		*/
		while (!audiobuffer_ready) Sleep(1);
		memcpy(buf2, audiobuffer, sizeof(float) * NUM_CHANNELS * FRAMES_PER_BUFFER); //we have
		//2024feb20, spi, end
		/*
		for(int i=0; i<1024; i++)
		{
			fftbuf[i]=0.0f;
			//fftbuf[i]=abs(buf2[i]);
		}
		*/

		fft(buf2, fftbuf, numberofsamples); // 1024);
		for (int i = 0; i < numberofsamples; i++) //1024; i++)
		{
			fftbuf[i] = abs(fftbuf[i]);
		}

		if (!specmode)
		{ // "normal" FFT
			memset(specbuf, 0, SPECWIDTH * SPECHEIGHT);
			for (x = 0; x < SPECWIDTH / 2; x++)
			{
				/*
#if 1
				y=sqrt(fftbuf[x+1])*3*SPECHEIGHT-4; // scale it (sqrt to make low values more visible)
#else
				y=fftbuf[x+1]*10*SPECHEIGHT; // scale it (linearly)
#endif
				*/
				//2024feb20, spi, begin
				int x_infft = 0;
				if (numberofsamples < SPECWIDTH)
				{
					x_infft = 1 + (int)((float)x * ((float)numberofsamples) / ((float)SPECWIDTH));
				}
				else
				{
					x_infft = 1 + x;
				}
				/*
				#if 1
					y = sqrt(_fftbuf[x + 1]) * 3 * _SPECHEIGHT - 4; // scale it (sqrt to make low values more visible)
				#else
					y = _fftbuf[x + 1] * 10 * _SPECHEIGHT; // scale it (linearly)
				#endif
				*/
				if (1)
				{
					y = sqrt(fftbuf[x_infft]) * 3 * SPECHEIGHT - 4; // scale it (sqrt to make low values more visible)
				}
				else
				{
					y = fftbuf[x_infft] * 10 * SPECHEIGHT; // scale it (linearly)
				}
				//2024feb20, spi, end

				if (y > SPECHEIGHT) y = SPECHEIGHT; // cap it
				if (x && (y1 = (y + y1) / 2)) // interpolate from previous to make the display smoother
					//while (--y1>=0) specbuf[y1*SPECWIDTH+x*2-1]=y1+1;
					while (--y1 >= 0) specbuf[y1 * SPECWIDTH + x * 2 - 1] = (127 * y1 / SPECHEIGHT) + 1;
				y1 = y;
				//while (--y>=0) specbuf[y*SPECWIDTH+x*2]=y+1; // draw level
				while (--y >= 0) specbuf[y * SPECWIDTH + x * 2] = (127 * y / SPECHEIGHT) + 1; // draw level
			}
		}
		else if (specmode == 1)
		{ // logarithmic, acumulate & average bins
			int b0 = 0;
			memset(specbuf, 0, SPECWIDTH * SPECHEIGHT);
			//#define BANDS 28
			//#define BANDS 80
			//#define BANDS 12
						//2024feb20, spi, begin
			if (global_bands > 2 && global_bands < SPECWIDTH)
			{
				//2024feb20, spi, end

				for (x = 0; x < global_bands; x++)
				{
					float peak = 0;
					int b1 = pow(2, x * 10.0 / (global_bands - 1));
					//2024feb20, spi, begin
					//if (b1 > 1023) b1 = 1023;
					if (b1 > (numberofsamples - 1)) b1 = (numberofsamples - 1);
					//2024feb20, spi, end
					if (b1 <= b0) b1 = b0 + 1; // make sure it uses at least 1 FFT bin
					for (; b0 < b1; b0++)
					{

						//find bin peak
						//2024feb20, spi, begin
						int b_infft = 0;
						if (numberofsamples < SPECWIDTH)
						{
							b_infft = 1 + (int)((float)b0 * ((float)numberofsamples) / ((float)SPECWIDTH));
						}
						else
						{
							b_infft = 1 + b0;
						}
						//if (peak < _fftbuf[1 + b0]) peak = _fftbuf[1 + b0];
						if (peak < fftbuf[b_infft]) peak = fftbuf[b_infft];
						//2024feb20, spi, end

					}
					y = sqrt(peak) * 3 * SPECHEIGHT - 4; // scale it (sqrt to make low values more visible)
					if (y > SPECHEIGHT) y = SPECHEIGHT; // cap it
					while (--y >= 0)
					{
						//memset(specbuf+y*SPECWIDTH+x*(SPECWIDTH/global_bands),y+1,SPECWIDTH/global_bands-2); // draw bar
						memset(specbuf + y * SPECWIDTH + x * (SPECWIDTH / global_bands), (127 * y / SPECHEIGHT) + 1, SPECWIDTH / global_bands - 2); // draw bar
					}
				}
				//2024feb20, spi, begin
			}
			//2024feb20, spi, end
		}
		else
		{
			// "3D"
			//2024feb20, spi, begin
			if (prev_specmode != specmode)
			{
				memset(specbuf, 0, SPECWIDTH * SPECHEIGHT);
			}
			//2024feb20, spi, end

			for (x = 0; x < SPECHEIGHT; x++)
			{
				//2024feb20, spi, begin
				//y = sqrt(_fftbuf[x + 1]) * 3 * 127; // scale it (sqrt to make low values more visible)
				int x_infft = 0;
				if (numberofsamples < SPECHEIGHT)
				{
					x_infft = 1 + (int)((float)x * ((float)numberofsamples) / ((float)SPECHEIGHT));
				}
				else
				{
					x_infft = 1 + x;
				}
				y = sqrt(fftbuf[x_infft]) * 3 * 127; // scale it (sqrt to make low values more visible)
				//2024feb20, spi, end
				if (y > 127) y = 127; // cap it
				specbuf[x * SPECWIDTH + specpos] = 128 + y; // plot it
			}
			// move marker onto next position
			specpos = (specpos + 1) % SPECWIDTH;
			for (x = 0; x < SPECHEIGHT; x++) specbuf[x * SPECWIDTH + specpos] = 255; //draws marker
		}
	}

	// update the display
	if (0)
	{
		dc = GetDC(global_hwnd); // dc = GetDC(win);
		BitBlt(dc, 0, 0, SPECWIDTH, SPECHEIGHT, specdc, 0, 0, SRCCOPY);
		ReleaseDC(global_hwnd, dc); // ReleaseDC(win, dc);
	}
	else
	{
		InvalidateRect(global_hwnd, NULL, FALSE);
	}
	//2024feb20, spi, begin
	prev_specmode = specmode;
	//2024feb20, spi, end

}
//2024mar10, spi, end





void CALLBACK StartGlobalProcess(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	//WavSetLib_Initialize(global_hwnd, IDC_MAIN_STATIC, global_staticwidth, global_staticheight, global_fontwidth, global_fontheight);
	//global_pfile = fopen("output.txt", "w");
	global_pfile = NULL;
	WavSetLib_Initialize(global_hwnd, IDC_MAIN_STATIC, global_staticwidth, global_staticheight, global_fontwidth, global_fontheight, global_staticalignment, global_pfile);

	//testing start /b, it does not work, like if /b has not effect
	//system("start /b c:\\app-bin\\sox\\sox.exe -q \"d:\\temp\\test.wav\" -d trim 0 10.0");
	//testing ShellExecute(), it works
	//ShellExecute(NULL, L"open", L"c:\\app-bin\\sox\\sox.exe", L"-q \"d:\\temp\\test.wav\" -d trim 0 10.0", NULL, 0);



	/////////////////////
	//initialize portmidi
	/////////////////////
    PmError err;
	Pm_Initialize(); 

	/////////////////////////////
	//input midi device selection
	/////////////////////////////
	const PmDeviceInfo* deviceInfo;
    int numDevices = Pm_CountDevices();
    for( int i=0; i<numDevices; i++ )
    {
        deviceInfo = Pm_GetDeviceInfo( i );
		if (deviceInfo->input)
		{
			string devicenamestring = deviceInfo->name;
			global_inputmididevicemap.insert(pair<string,int>(devicenamestring,i));
		}
	}
	std::map<string,int>::iterator it;
	it = global_inputmididevicemap.find(global_inputmididevicename);
	if(it!=global_inputmididevicemap.end())
	{
		global_inputmidideviceid = (*it).second;
		if (global_textdisplay) { sprintf(pCHAR, "%s maps to %d\n", global_inputmididevicename.c_str(), global_inputmidideviceid); StatusAddTextA(pCHAR); }
		deviceInfo = Pm_GetDeviceInfo(global_inputmidideviceid);
	}
	else
	{
		assert(false);
		for(it=global_inputmididevicemap.begin(); it!=global_inputmididevicemap.end(); it++)
		{
			sprintf(pCHAR, "%s maps to %d\n", (*it).first.c_str(), (*it).second);StatusAddTextA(pCHAR);
		}
		swprintf(pWCHAR, L"input midi device not found\n");StatusAddText(pWCHAR);
		return;
	}

    // use porttime callback to empty midi queue and print 
    Pt_Start(1, receive_poll, 0); //Pt_Start(1, receive_poll, global_pInstrument); 
    // list device information 
	if (global_textdisplay) { swprintf(pWCHAR, L"MIDI input devices:\n"); StatusAddText(pWCHAR); }
    for (int i = 0; i < Pm_CountDevices(); i++) 
	{
        const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
        if (info->input) 
		{
			if (global_textdisplay) { sprintf(pCHAR, "%d: %s, %s\n", i, info->interf, info->name); StatusAddTextA(pCHAR); }
		}
    }
    //inputmididevice = get_number("Type input device number: ");
	if (global_textdisplay) { swprintf(pWCHAR, L"device %d selected\n", global_inputmidideviceid); StatusAddText(pWCHAR); }

    err = Pm_OpenInput(&global_pPmStreamMIDIIN, global_inputmidideviceid, NULL, 512, NULL, NULL);
    if (err) 
	{
        sprintf(pCHAR, Pm_GetErrorText(err));StatusAddTextA(pCHAR);
        Pt_Stop();
		//Terminate();
        //mmexit(1);
		return;
    }
    Pm_SetFilter(global_pPmStreamMIDIIN, filter);
    global_inited = true; // now can document changes, set filter 
	if (global_textdisplay) { swprintf(pWCHAR, L"spitonicmidiinstrumentsynthwin32 ready.\n"); StatusAddText(pWCHAR); }
    global_active = true;

	/*
	//1) load a sample file
	SndfileHandle file1;
	//file1 = SndfileHandle("D:\\oifii-org\\httpdocs\\ha-org\\had\\dj-oifii\\worldaudio_wav\\00min15sec-and-less\\Geoffrey Oryema - TAO -  mara(introlater)_9sec.wav");
	file1 = SndfileHandle("D:\\oifii-org\\httpdocs\\ha-org\\had\\dj-oifii\\worldaudio_wav\\00min30sec-and-less\\GF - Subramanian - track 03(intro)_18sec.wav");
	assert(file1.samplerate() == 44100);
	assert(file1.channels() == 2);
	float file1duration_s = ((float)file1.frames()) / ((float)file1.samplerate());
	while (0)
	{
		//terminate portmidi
		//global_active = false;
		//Pm_Close(global_pPmStreamMIDIIN);
		//Pt_Stop();
		//Pm_Terminate();

		//play synth automatically
		int random_integer;
		int lowest = 36, highest = 84;
		int range = (highest - lowest) + 1;
		random_integer = lowest + int(range*rand() / (RAND_MAX + 1.0));
		//int midinotenumber = 64; //range 0 to 127
		int midinotenumber = random_integer; //range 0 to 127
		//int midinotevelocity = 127; //range 0 to 127
		lowest = 10; highest = 127;
		random_integer = lowest + int(range*rand() / (RAND_MAX + 1.0));
		int midinotevelocity = random_integer; //range 0 to 127
		if (midinotevelocity != 0)
		{
			swprintf(pWCHAR, L"auto Note %d , Velocity %d\n", midinotenumber, midinotevelocity); StatusAddText(pWCHAR);

			//2.1) set a parameter that we created when we defined the synth
			synth.setParameter("midiNote", midinotenumber);
			synth.setParameter("midiNoteVelocity", midinotevelocity);
			//2.2) trigger note
			//simply setting the value of a parameter causes that parameter to send a "trigger" message to any using them as triggers
			synth.setParameter("trigger", 1);
		}


		Sleep(file1duration_s * 1000);
	}
	//PostMessage(global_hwnd, WM_DESTROY, 0, 0);
	*/
}



//2024mar10, spi, begin
/*
PCHAR*
    CommandLineToArgvA(
        PCHAR CmdLine,
        int* _argc
        )
    {
        PCHAR* argv;
        PCHAR  _argv;
        ULONG   len;
        ULONG   argc;
        CHAR   a;
        ULONG   i, j;

        BOOLEAN  in_QM;
        BOOLEAN  in_TEXT;
        BOOLEAN  in_SPACE;

        len = strlen(CmdLine);
        i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

        argv = (PCHAR*)GlobalAlloc(GMEM_FIXED,
            i + (len+2)*sizeof(CHAR));

        _argv = (PCHAR)(((PUCHAR)argv)+i);

        argc = 0;
        argv[argc] = _argv;
        in_QM = FALSE;
        in_TEXT = FALSE;
        in_SPACE = TRUE;
        i = 0;
        j = 0;

        while( a = CmdLine[i] ) {
            if(in_QM) {
                if(a == '\"') {
                    in_QM = FALSE;
                } else {
                    _argv[j] = a;
                    j++;
                }
            } else {
                switch(a) {
                case '\"':
                    in_QM = TRUE;
                    in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    in_SPACE = FALSE;
                    break;
                case ' ':
                case '\t':
                case '\n':
                case '\r':
                    if(in_TEXT) {
                        _argv[j] = '\0';
                        j++;
                    }
                    in_TEXT = FALSE;
                    in_SPACE = TRUE;
                    break;
                default:
                    in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    _argv[j] = a;
                    j++;
                    in_SPACE = FALSE;
                    break;
                }
            }
            i++;
        }
        _argv[j] = '\0';
        argv[argc] = NULL;

        (*_argc) = argc;
        return argv;
    }
*/
//2024mar10, spi, end


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	//LPWSTR *szArgList;
	LPSTR *szArgList;
	int nArgs;
	int i;

	//szArgList = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	szArgList = CommandLineToArgvA(GetCommandLineA(), &nArgs);
	if( NULL == szArgList )
	{
		//wprintf(L"CommandLineToArgvW failed\n");
		return FALSE;
	}
	LPWSTR *szArgListW;
	int nArgsW;
	szArgListW = CommandLineToArgvW(GetCommandLineW(), &nArgsW);
	if( NULL == szArgListW )
	{
		//wprintf(L"CommandLineToArgvW failed\n");
		return FALSE;
	}

	if(nArgs>1)
	{
		//inputmididevice=atoi(argv[2]);
		global_inputmididevicename=szArgList[1]; //"Q49", "In From MIDI Yoke:  1", "In From MIDI Yoke:  2", ... , "In From MIDI Yoke:  8"
	}
	//int inputmididevice =  11; //alesis q49 midi port id (when midi yoke installed)
	//int inputmididevice =  1; //midi yoke 1 (when midi yoke installed)
	if(nArgs>2)
	{
		global_inputmidichannel=atoi(szArgList[2]);
	}

	mySPIAudioDevice.global_audiooutputdevicename = "E-MU ASIO"; //"Wave (2- E-MU E-DSP Audio Proce"
	if (nArgs>3)
	{
		//global_filename = szArgList[1];
		mySPIAudioDevice.global_audiooutputdevicename = szArgList[3];
	}
	mySPIAudioDevice.global_outputAudioChannelSelectors[0] = 0; // on emu patchmix ASIO device channel 1 (left)
	mySPIAudioDevice.global_outputAudioChannelSelectors[1] = 1; // on emu patchmix ASIO device channel 2 (right)
	//global_outputAudioChannelSelectors[0] = 2; // on emu patchmix ASIO device channel 3 (left)
	//global_outputAudioChannelSelectors[1] = 3; // on emu patchmix ASIO device channel 4 (right)
	//global_outputAudioChannelSelectors[0] = 8; // on emu patchmix ASIO device channel 9 (left)
	//global_outputAudioChannelSelectors[1] = 9; // on emu patchmix ASIO device channel 10 (right)
	//global_outputAudioChannelSelectors[0] = 10; // on emu patchmix ASIO device channel 11 (left)
	//global_outputAudioChannelSelectors[1] = 11; // on emu patchmix ASIO device channel 12 (right)
	if (nArgs>4)
	{
		mySPIAudioDevice.global_outputAudioChannelSelectors[0] = atoi((LPCSTR)(szArgList[4])); //0 for first asio channel (left) or 2, 4, 6, etc.
	}
	if (nArgs>5)
	{
		mySPIAudioDevice.global_outputAudioChannelSelectors[1] = atoi((LPCSTR)(szArgList[5])); //1 for second asio channel (right) or 3, 5, 7, etc.
	}


	if(nArgs>6)
	{
		global_x = atoi(szArgList[6]);
	}
	if(nArgs>7)
	{
		global_y = atoi(szArgList[7]);
	}
	if(nArgs>8)
	{
		global_xwidth = atoi(szArgList[8]);
	}
	if(nArgs>9)
	{
		global_yheight = atoi(szArgList[9]);
	}
	if(nArgs>10)
	{
		global_alpha = atoi(szArgList[10]);
	}
	if(nArgs>11)
	{
		global_titlebardisplay = atoi(szArgList[11]);
	}
	if(nArgs>12)
	{
		global_menubardisplay = atoi(szArgList[12]);
	}
	if(nArgs>13)
	{
		global_acceleratoractive = atoi(szArgList[13]);
	}
	if(nArgs>14)
	{
		global_fontheight = atoi(szArgList[14]);
	}
	if(nArgs>15)
	{
		global_fontcolor_r = atoi(szArgList[15]);
	}
	if(nArgs>16)
	{
		global_fontcolor_g = atoi(szArgList[16]);
	}
	if(nArgs>17)
	{
		global_fontcolor_b = atoi(szArgList[17]);
	}
	if(nArgs>18)
	{
		global_staticalignment = atoi(szArgList[18]);
	}
	//new parameters
	if(nArgs>19)
	{
		wcscpy(szWindowClass, szArgListW[19]); 
	}
	if(nArgs>20)
	{
		wcscpy(szTitle, szArgListW[20]); 
	}
	if(nArgs>21)
	{
		global_begin = szArgList[21]; 
	}
	if(nArgs>22)
	{
		global_end = szArgList[22]; 
	}
	//2024mar10, spi, begin
	if (nArgs > 23)
	{
		global_soundnumber = atoi(szArgList[23]); //between 1 and 16
	}
	if (nArgs > 24)
	{
		global_textdisplay = atoi(szArgList[24]); //0 or 1 for disabled or enabled
	}
	if (nArgs > 25)
	{
		global_spectrumdisplay = atoi(szArgList[25]); //0 or 1 for disabled or enabled
	}
	if (nArgs > 26)
	{
		global_idcolorpalette = atoi(szArgList[26]); //between 0 and 7
	}
	if (nArgs > 27)
	{
		specmode = atoi(szArgList[27]); //between 0 and 18
	}
	//2024mar10, spi, end
	LocalFree(szArgList);
	LocalFree(szArgListW);

	int nShowCmd = false;
	//ShellExecuteA(NULL, "open", "begin.bat", "", NULL, nShowCmd);
	ShellExecuteA(NULL, "open", global_begin.c_str(), "", NULL, nCmdShow);


	//////////////////////////
	//initialize random number
	//////////////////////////
	srand((unsigned)time(0));


	pFILE = fopen("devices.txt", "w");

	///////////////////////
	//initialize port audio
	///////////////////////
	mySPIAudioDevice.global_err = Pa_Initialize();
	if (mySPIAudioDevice.global_err != paNoError)
	{
		//MessageBox(0,"portaudio initialization failed",0,MB_ICONERROR);
		if (pFILE) fprintf(pFILE, "portaudio initialization failed.\n");
		fclose(pFILE);
		return 1;
	}

	////////////////////////
	//audio device selection
	////////////////////////
	mySPIAudioDevice.m_pFILE = pFILE;
	//mySPIAudioDevice.SelectAudioInputDevice();
	mySPIAudioDevice.SelectAudioOutputDevice();
	fclose(mySPIAudioDevice.m_pFILE);
	mySPIAudioDevice.m_pFILE = NULL;

	///////////////////////
	//set tonic sample rate 
	///////////////////////
	// You don't necessarily have to do this - it will default to 44100 if not set.
	Tonic::setSampleRate(SAMPLE_RATE);

	//2024mar10, spi, begin
	if (global_soundnumber <= 0 || global_soundnumber == 1)
	{
		poly.addVoices(createSynthVoice, 8); //basic synth
	}
	else if (global_soundnumber == 2)
	{
		poly.addVoices(createSynthVoice_v2, 8); //control switcher synth
	}
	else if (global_soundnumber == 3)
	{
		poly.addVoices(createSynthVoice_v3, 8); //arbitrary table lookup synth
	}
	else if (global_soundnumber == 4)
	{
		poly.addVoices(createSynthVoice_v4, 8); //events bufferplayer synth (under construction)
	}
	else if (global_soundnumber == 5)
	{
		poly.addVoices(createSynthVoice_v5, 8); //band limited oscillator synth
	}
	else if (global_soundnumber == 6)
	{
		poly.addVoices(createSynthVoice_v6, 8); //compressor synth
	}
	else if (global_soundnumber == 7)
	{
		poly.addVoices(createSynthVoice_v7, 8); //control snap to scale synth (under construction)
	}
	else if (global_soundnumber == 8)
	{
		poly.addVoices(createSynthVoice_v8, 8); //delay synth
	}
	else if (global_soundnumber == 9)
	{
		poly.addVoices(createSynthVoice_v9, 8); //events synth
	}
	else if (global_soundnumber == 10)
	{
		poly.addVoices(createSynthVoice_v10, 8); //filtered noise synth
	}
	else if (global_soundnumber == 11)
	{
		poly.addVoices(createSynthVoice_v11, 8); //filter synth (under construction)
	}
	else if (global_soundnumber == 12)
	{
		poly.addVoices(createSynthVoice_v12, 8); //fm drone synth
	}
	else if (global_soundnumber == 13)
	{
		poly.addVoices(createSynthVoice_v13, 8); //lf noise synth
	}
	else if (global_soundnumber == 14)
	{
		poly.addVoices(createSynthVoice_v14, 8); //reverb synth
	}
	else if (global_soundnumber == 15)
	{
		poly.addVoices(createSynthVoice_v15, 8); //simple step seq synth (under construction) 
	}
	else if (global_soundnumber >= 16)
	{
		poly.addVoices(createSynthVoice_v16, 8); //sine sum synth
	}
	//2024mar10, spi, end

	StereoDelay delay = StereoDelay(3.0f, 3.0f)
		.delayTimeLeft(0.25 + SineWave().freq(0.2) * 0.01)
		.delayTimeRight(0.30 + SineWave().freq(0.23) * 0.01)
		.feedback(0.4)
		.dryLevel(0.8)
		.wetLevel(0.2);

	synth.setOutputGen(poly >> delay);


	//////////////
	//setup stream  
	//////////////
	mySPIAudioDevice.global_err = Pa_OpenStream(
		&mySPIAudioDevice.global_stream,
		NULL, //NULL, //&global_inputParameters,
		&mySPIAudioDevice.global_outputParameters,
		SAMPLE_RATE,
		FRAMES_PER_BUFFER,
		0, //paClipOff,      // we won't output out of range samples so don't bother clipping them
		renderCallback,
		NULL); //no callback userData
	if (mySPIAudioDevice.global_err != paNoError)
	{
		char errorbuf[2048];
		sprintf(errorbuf, "Unable to open stream: %s\n", Pa_GetErrorText(mySPIAudioDevice.global_err));
		//MessageBox(0,errorbuf,0,MB_ICONERROR);
		if (pFILE) fprintf(pFILE, "%s\n", errorbuf);
		fclose(pFILE);
		return 1;
	}



	//////////////
	//start stream  
	//////////////
	mySPIAudioDevice.global_err = Pa_StartStream(mySPIAudioDevice.global_stream);
	if (mySPIAudioDevice.global_err != paNoError)
	{
		char errorbuf[2048];
		sprintf(errorbuf, "Unable to start stream: %s\n", Pa_GetErrorText(mySPIAudioDevice.global_err));
		//MessageBox(0,errorbuf,0,MB_ICONERROR);
		if (pFILE) fprintf(pFILE, "%s\n", errorbuf);
		fclose(pFILE);
		return 1;
	}



	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	//LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	//LoadString(hInstance, IDC_SPIWAVWIN32, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	if(global_acceleratoractive)
	{
		hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SPIWAVWIN32));
	}
	else
	{
		hAccelTable = NULL;
	}
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	//wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SPIWAVWIN32));
	wcex.hIcon			= (HICON)LoadImage(NULL, L"background_32x32x16.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);

	if(global_menubardisplay)
	{
		wcex.lpszMenuName = MAKEINTRESOURCE(IDC_SPIWAVWIN32); //original with menu
	}
	else
	{
		wcex.lpszMenuName = NULL; //no menu
	}
	wcex.lpszClassName	= szWindowClass;
	//wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	wcex.hIconSm		= (HICON)LoadImage(NULL, L"background_16x16x16.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	global_dib = FreeImage_Load(FIF_JPEG, "background.jpg", JPEG_DEFAULT);


	FIBITMAP* local_16x16xrgbdib = FreeImage_Rescale(global_dib, 16, 16, FILTER_BICUBIC);
	FreeImage_Save(FIF_ICO, local_16x16xrgbdib, "background_16x16xrgb-new.ico");
	FreeImage_Unload(local_16x16xrgbdib);

	FIBITMAP* local_32x32xrgbdib = FreeImage_Rescale(global_dib, 32, 32, FILTER_BICUBIC);
	FreeImage_Save(FIF_ICO, local_32x32xrgbdib, "background_32x32xrgb-new.ico");
	FreeImage_Unload(local_32x32xrgbdib);

	FIBITMAP* local_48x48xrgbdib = FreeImage_Rescale(global_dib, 48, 48, FILTER_BICUBIC);
	FreeImage_Save(FIF_ICO, local_48x48xrgbdib, "background_48x48xrgb-new.ico");
	FreeImage_Unload(local_48x48xrgbdib);


	//global_hFont=CreateFontW(32,0,0,0,FW_BOLD,0,0,0,0,0,0,2,0,L"SYSTEM_FIXED_FONT");
	global_hFont=CreateFontW(global_fontheight,0,0,0,FW_NORMAL,0,0,0,0,0,0,2,0,L"SYSTEM_FIXED_FONT");

	if(global_titlebardisplay)
	{
		hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, //original with WS_CAPTION etc.
			global_x, global_y, global_xwidth, global_yheight, NULL, NULL, hInstance, NULL);
	}
	else
	{
		hWnd = CreateWindow(szWindowClass, szTitle, WS_POPUP | WS_VISIBLE, //no WS_CAPTION etc.
			global_x, global_y, global_xwidth, global_yheight, NULL, NULL, hInstance, NULL);
	}
	if (!hWnd)
	{
		return FALSE;
	}
	global_hwnd = hWnd;

	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hWnd, 0, global_alpha, LWA_ALPHA);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return TRUE;
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	HGDIOBJ hOldBrush;
	HGDIOBJ hOldPen;
	int iOldMixMode;
	COLORREF crOldBkColor;
	COLORREF crOldTextColor;
	int iOldBkMode;
	HFONT hOldFont, hFont;
	TEXTMETRIC myTEXTMETRIC;

	switch (message)
	{
	case WM_CREATE:
		{
			//HWND hStatic = CreateWindowEx(WS_EX_TRANSPARENT, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_CENTER,  
			HWND hStatic = CreateWindowEx(WS_EX_TRANSPARENT, L"STATIC", L"", WS_CHILD | WS_VISIBLE | global_staticalignment, 
				0, 100, 100, 100, hWnd, (HMENU)IDC_MAIN_STATIC, GetModuleHandle(NULL), NULL);
			if(hStatic == NULL)
				MessageBox(hWnd, L"Could not create static text.", L"Error", MB_OK | MB_ICONERROR);
			SendMessage(hStatic, WM_SETFONT, (WPARAM)global_hFont, MAKELPARAM(FALSE, 0));



			global_timer=timeSetEvent(1000,25,(LPTIMECALLBACK)&StartGlobalProcess,0,TIME_ONESHOT);
		}
		break;
	case WM_SIZE:
		{
			RECT rcClient;

			GetClientRect(hWnd, &rcClient);
			/*
			HWND hEdit = GetDlgItem(hWnd, IDC_MAIN_EDIT);
			SetWindowPos(hEdit, NULL, 0, 0, rcClient.right/2, rcClient.bottom/2, SWP_NOZORDER);
			*/
			HWND hStatic = GetDlgItem(hWnd, IDC_MAIN_STATIC);
			global_staticwidth = rcClient.right - 0;
			//global_staticheight = rcClient.bottom-(rcClient.bottom/2);
			global_staticheight = rcClient.bottom - 0;

			//spi, begin
			global_imagewidth = rcClient.right - 0;
			global_imageheight = rcClient.bottom - 0; 
			WavSetLib_Initialize(global_hwnd, IDC_MAIN_STATIC, global_staticwidth, global_staticheight, global_fontwidth, global_fontheight, global_staticalignment, global_pfile);
			//spi, end
			//SetWindowPos(hStatic, NULL, 0, rcClient.bottom/2, global_staticwidth, global_staticheight, SWP_NOZORDER);
			SetWindowPos(hStatic, NULL, 0, 0, global_staticwidth, global_staticheight, SWP_NOZORDER);

			//2024mar10, spi, begin
			if (global_spectrumdisplay)
			{
				if ((global_imagewidth == SPECWIDTH) && (global_imageheight == SPECHEIGHT) && specbmp != 0)
				{
					//already allocated and window size not changing
					//do nothing
				}
				else
				{
					if (timer) { timeKillEvent(timer); Sleep(timer_delay_ms); } //sleep to let previous timer finish its drawing
					if (specdc) DeleteDC(specdc);
					if (specbmp) DeleteObject(specbmp);

					if (0) { global_idcolorpalette = RandomInt(0, 7); } //for testing all these palette modes

					SPECWIDTH = global_imagewidth;
					int remainder = SPECWIDTH % 4;
					SPECWIDTH = SPECWIDTH - remainder;
					SPECHEIGHT = global_imageheight;
					{ // create bitmap to draw spectrum in (8 bit for easy updating)
						BYTE data[2000] = { 0 };
						BITMAPINFOHEADER* bh = (BITMAPINFOHEADER*)data;
						RGBQUAD* pal = (RGBQUAD*)(data + sizeof(*bh));
						int a;
						bh->biSize = sizeof(*bh);
						bh->biWidth = SPECWIDTH;
						bh->biHeight = SPECHEIGHT; // upside down (line 0=bottom)
						bh->biPlanes = 1;
						bh->biBitCount = 8;
						bh->biClrUsed = bh->biClrImportant = 256;
						// setup palette

						if (global_idcolorpalette == 0)
						{
							//original palette, green shifting to red
							for (a = 1; a < 128; a++) {
								pal[a].rgbGreen = 256 - 2 * a;
								pal[a].rgbRed = 2 * a;
							}
							for (a = 0; a < 32; a++) {
								pal[128 + a].rgbBlue = 8 * a;
								pal[128 + 32 + a].rgbBlue = 255;
								pal[128 + 32 + a].rgbRed = 8 * a;
								pal[128 + 64 + a].rgbRed = 255;
								pal[128 + 64 + a].rgbBlue = 8 * (31 - a);
								pal[128 + 64 + a].rgbGreen = 8 * a;
								pal[128 + 96 + a].rgbRed = 255;
								pal[128 + 96 + a].rgbGreen = 255;
								pal[128 + 96 + a].rgbBlue = 8 * a;
							}
						}
						else if (global_idcolorpalette == 1)
						{
							//altered palette, red shifting to green
							for (a = 1; a < 128; a++) {
								pal[a].rgbRed = 256 - 2 * a;
								pal[a].rgbGreen = 2 * a;
							}
							for (a = 0; a < 32; a++) {
								pal[128 + a].rgbBlue = 8 * a;
								pal[128 + 32 + a].rgbBlue = 255;
								pal[128 + 32 + a].rgbGreen = 8 * a;
								pal[128 + 64 + a].rgbGreen = 255;
								pal[128 + 64 + a].rgbBlue = 8 * (31 - a);
								pal[128 + 64 + a].rgbRed = 8 * a;
								pal[128 + 96 + a].rgbGreen = 255;
								pal[128 + 96 + a].rgbRed = 255;
								pal[128 + 96 + a].rgbBlue = 8 * a;
							}
						}
						else if (global_idcolorpalette == 2)
						{
							//altered palette, blue shifting to green
							for (a = 1; a < 128; a++) {
								pal[a].rgbBlue = 256 - 2 * a;
								pal[a].rgbGreen = 2 * a;
							}
							for (a = 0; a < 32; a++) {
								pal[128 + a].rgbBlue = 8 * a;
								pal[128 + 32 + a].rgbRed = 255;
								pal[128 + 32 + a].rgbGreen = 8 * a;
								pal[128 + 64 + a].rgbGreen = 255;
								pal[128 + 64 + a].rgbRed = 8 * (31 - a);
								pal[128 + 64 + a].rgbBlue = 8 * a;
								pal[128 + 96 + a].rgbGreen = 255;
								pal[128 + 96 + a].rgbBlue = 255;
								pal[128 + 96 + a].rgbRed = 8 * a;
							}
						}
						else if (global_idcolorpalette == 3)
						{
							//altered palette, black shifting to white - grascale
							for (a = 1; a < 256; a++) {
								pal[a].rgbRed = a;
								pal[a].rgbBlue = a;
								pal[a].rgbGreen = a;
							}
						}
						else if (global_idcolorpalette == 4)
						{
							//altered palette, pink
							for (a = 1; a < 256; a++) {
								pal[a].rgbRed = 255;
								pal[a].rgbBlue = 255;
								pal[a].rgbGreen = a;
							}
						}
						else if (global_idcolorpalette == 5)
						{
							//altered palette, yellow
							for (a = 1; a < 256; a++) {
								pal[a].rgbRed = 255;
								pal[a].rgbBlue = a;
								pal[a].rgbGreen = 255;
							}
						}
						else if (global_idcolorpalette == 6)
						{
							//altered palette, cyan
							for (a = 1; a < 256; a++) {
								pal[a].rgbRed = a;
								pal[a].rgbBlue = 255;
								pal[a].rgbGreen = 255;
							}
						}
						else if (global_idcolorpalette == 7)
						{
							//altered palette, lite green
							for (a = 1; a < 256; a++) {
								pal[a].rgbRed = a;
								pal[a].rgbBlue = 127;
								pal[a].rgbGreen = 255;
							}
						}


						// create the bitmap
						specbmp = CreateDIBSection(0, (BITMAPINFO*)bh, DIB_RGB_COLORS, (void**)&specbuf, NULL, 0);
						specdc = CreateCompatibleDC(0);
						SelectObject(specdc, specbmp);
					}
					// setup update timer (40hz)
					//timer = timeSetEvent(25, 25, (LPTIMECALLBACK)&UpdateSpectrum, 0, TIME_PERIODIC);
					timer = timeSetEvent(timer_delay_ms, 25, (LPTIMECALLBACK)&UpdateSpectrum, 0, TIME_PERIODIC);
				}

			}
			//2024mar10, spi, end
		}
		break;
	//2024mar10, spi, begin
	case WM_LBUTTONUP:
		{
			specmode = (specmode + 1) % 19; // swap spectrum mode
			memset(specbuf, 0, SPECWIDTH* SPECHEIGHT);	// clear display
			return 0;
		}
		break;
	case WM_RBUTTONUP:
		{
			specmode = (specmode - 1); // swap spectrum mode
			if (specmode < 0) specmode = 19 - 1;
			memset(specbuf, 0, SPECWIDTH * SPECHEIGHT);	// clear display
			return 0;
		}
		break;
	//2024mar10, spi, end
	case WM_CTLCOLOREDIT:
		{
			SetBkMode((HDC)wParam, TRANSPARENT);
			SetTextColor((HDC)wParam, RGB(0xFF, 0xFF, 0xFF));
			return (INT_PTR)::GetStockObject(NULL_PEN);
		}
		break;
	case WM_CTLCOLORSTATIC:
		{
			SetBkMode((HDC)wParam, TRANSPARENT);
			//SetTextColor((HDC)wParam, RGB(0xFF, 0xFF, 0xFF));
			SetTextColor((HDC)wParam, RGB(global_fontcolor_r, global_fontcolor_g, global_fontcolor_b));
			return (INT_PTR)::GetStockObject(NULL_PEN);
		}
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		//spi, begin
		
		
		//2024mar10, spi, begin
		if (global_spectrumdisplay)
		{
			BitBlt(hdc, 0, 0, SPECWIDTH, SPECHEIGHT, specdc, 0, 0, SRCCOPY);
		}
		else 
		{
			SetStretchBltMode(hdc, COLORONCOLOR);

			StretchDIBits(hdc, 0, 0, global_imagewidth, global_imageheight,
				0, 0, FreeImage_GetWidth(global_dib), FreeImage_GetHeight(global_dib),
				FreeImage_GetBits(global_dib), FreeImage_GetInfo(global_dib), DIB_RGB_COLORS, SRCCOPY);
		}
		//2024mar10, spi, end


		//spi, end
		hOldBrush = SelectObject(hdc, (HBRUSH)GetStockObject(GRAY_BRUSH));
		hOldPen = SelectObject(hdc, (HPEN)GetStockObject(WHITE_PEN));
		//iOldMixMode = SetROP2(hdc, R2_MASKPEN);
		iOldMixMode = SetROP2(hdc, R2_MERGEPEN);
		//Rectangle(hdc, 100, 100, 200, 200);

		crOldBkColor = SetBkColor(hdc, RGB(0xFF, 0x00, 0x00));
		crOldTextColor = SetTextColor(hdc, RGB(0xFF, 0xFF, 0xFF));
		iOldBkMode = SetBkMode(hdc, TRANSPARENT);
		//hFont=CreateFontW(70,0,0,0,FW_BOLD,0,0,0,0,0,0,2,0,L"SYSTEM_FIXED_FONT");
		//hOldFont=(HFONT)SelectObject(hdc,global_hFont);
		hOldFont=(HFONT)SelectObject(hdc,global_hFont);
		GetTextMetrics(hdc, &myTEXTMETRIC);
		global_fontwidth = myTEXTMETRIC.tmAveCharWidth;
		//TextOutW(hdc, 100, 100, L"test string", 11);

		SelectObject(hdc, hOldBrush);
		SelectObject(hdc, hOldPen);
		SetROP2(hdc, iOldMixMode);
		SetBkColor(hdc, crOldBkColor);
		SetTextColor(hdc, crOldTextColor);
		SetBkMode(hdc, iOldBkMode);
		SelectObject(hdc,hOldFont);
		//DeleteObject(hFont);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		{
			//2024mar10, spi, begin
			if (timer) timeKillEvent(timer); //update spectrum timer
			if (specdc) DeleteDC(specdc);
			if (specbmp) DeleteObject(specbmp);
			//2024mar10, spi, end 
			
			//terminate portmidi
			global_active = false;
			Pm_Close(global_pPmStreamMIDIIN);
			Pt_Stop();
			Pm_Terminate();
			//spi, begin
			/////////////////////
			//terminate portaudio
			/////////////////////
			mySPIAudioDevice.global_err = Pa_StopStream(mySPIAudioDevice.global_stream);
			if (mySPIAudioDevice.global_err != paNoError)
			{
				char errorbuf[2048];
				sprintf(errorbuf, "Error stoping stream: %s\n", Pa_GetErrorText(mySPIAudioDevice.global_err));
				MessageBoxA(0, errorbuf, 0, MB_ICONERROR);
				return 1;
			}
			mySPIAudioDevice.global_err = Pa_CloseStream(mySPIAudioDevice.global_stream);
			if (mySPIAudioDevice.global_err != paNoError)
			{
				char errorbuf[2048];
				sprintf(errorbuf, "Error closing stream: %s\n", Pa_GetErrorText(mySPIAudioDevice.global_err));
				MessageBoxA(0, errorbuf, 0, MB_ICONERROR);
				return 1;
			}
			Pa_Terminate();
			//spi, end
			//delete all memory allocations
			//if(global_pInstrument) delete global_pInstrument;
			//close file
			if(global_pfile) fclose(global_pfile);
			if (pFILE) fclose(pFILE); //added by spi

			//terminate wavset library
			WavSetLib_Terminate();
			//terminate win32 app.
			if (global_timer) timeKillEvent(global_timer);
			FreeImage_Unload(global_dib);
			DeleteObject(global_hFont);

			int nShowCmd = false;
			//ShellExecuteA(NULL, "open", "end.bat", "", NULL, nShowCmd);
			ShellExecuteA(NULL, "open", global_end.c_str(), "", NULL, 0);
			PostQuitMessage(0);
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
