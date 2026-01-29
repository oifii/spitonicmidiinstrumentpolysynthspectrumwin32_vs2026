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
 //2022nov14, spi, begin
#include "stdafx.h"
//2022nov14, spi, end

#include "portaudio.h"

//2022oct11, spi, begin
/*
#ifdef WIN32
#if PA_USE_ASIO
#include "pa_asio.h"
#endif
*/
#if (defined(_WIN64) || defined(_WIN32))
#if PA_USE_ASIO
#include "pa_asio.h"
#endif
#endif
//2022oct11, spi, end

#include "defs.h"

#include "spiaudiodevice.h"

#include <algorithm>

//2020dec19, spi, begin
//audio data stream (content) related
//extern int global_samplerate; //SAMPLE_RATE;
//extern int global_frames_per_buffer; //FRAMES_PER_BUFFER;
extern int global_numchannels; //NUM_CHANNELS;
//extern int global_sf_format; //SF_FORMAT_WAV | SF_FORMAT_PCM_16;
//2020dec19, spi, end


SPIAudioDevice::SPIAudioDevice()
{
	global_audioinputdevicename = "";
	global_audiooutputdevicename = "";
	//2020dec16, spi, begin
	global_audioinputhostapi = ""; 
	global_audiooutputhostapi = ""; 
	//2020dec16, spi, end

	//2022oct11, spi, begin
	bdetailedreport=true;
	//2022oct11, spi, end
	m_pFILE = NULL;
	//user must call it
	//ScanAudioDevices("", spiaudiodeviceALL);
}

SPIAudioDevice::~SPIAudioDevice()
{
	
}

int SPIAudioDevice::MatchHostAPI(string matchmode/* = ""*/, spiaudiodevicetypeflag ioflag/* =spiaudiodeviceALL*/)
{
	int hostapitype = -1; //defaults to not specified //could also force default to paMME;
	int hostapitypefound = -1; //not yet matched

	if (ioflag != spiaudiodeviceALL)
	{
		string audiohostapi = "";
		if (ioflag == spiaudiodeviceINPUT) audiohostapi = global_audioinputhostapi;
		else if(ioflag==spiaudiodeviceOUTPUT) audiohostapi = global_audiooutputhostapi;
		std::transform(audiohostapi.begin(), audiohostapi.end(), audiohostapi.begin(), ::toupper);

		if (!audiohostapi.empty())
		{
			string audiohostapifound = "";
			std::map<int, string>::iterator it;
			for (it = global_hostapimap_hostapitype.begin(); it != global_hostapimap_hostapitype.end(); it++)
			{
				string mystring = it->second;
				size_t found1 = audiohostapi.find(mystring);
				size_t found2 = mystring.find(audiohostapi);
				if ((found1 != std::string::npos) || (found2 != std::string::npos))
				{
					if (hostapitypefound == -1)
					{
						//it->second    // string
						audiohostapifound = it->second;
						//it->first   // value (key) 
						hostapitypefound = it->first;
						if (m_pFILE) fprintf(m_pFILE, "success: matching hostapi found, %s will be retained.\n", audiohostapifound.c_str());
					}
					else
					{
						//error, but will return safely with first matching hostapi
						if (m_pFILE) fprintf(m_pFILE, "warning: more than one matching hostapi found, %s will be ignored.\n", mystring.c_str());
						break;
					}
					if (m_pFILE) fflush(m_pFILE);
				}
			}
			if (hostapitypefound != -1) hostapitype = hostapitypefound;
		}
	}
	if (m_pFILE && (hostapitype == -1))
	{
		if (m_pFILE && (ioflag == spiaudiodeviceINPUT)) fprintf(m_pFILE, "warning: input hostapi not specified.\n");
		if (m_pFILE && (ioflag == spiaudiodeviceOUTPUT)) fprintf(m_pFILE, "warning: output hostapi not specified.\n");
		if (m_pFILE) fflush(m_pFILE);
	}
	return hostapitype;

}

int SPIAudioDevice::MatchDevice(string matchmode/* = ""*/, spiaudiodevicetypeflag ioflag/* =spiaudiodeviceALL*/, int hostapitype/*=-1*/)
{
	int deviceid = paNoDevice; //defaults to not specified
	int deviceidfound = -1; //not yet matched

	if (ioflag != spiaudiodeviceALL)
	{
		string audiodevicename = "";
		if (ioflag == spiaudiodeviceINPUT) audiodevicename = global_audioinputdevicename;
		else if (ioflag == spiaudiodeviceOUTPUT) audiodevicename = global_audiooutputdevicename;
		//std::transform(audiodevicename.begin(), audiodevicename.end(), audiodevicename.begin(), ::toupper);

		if (!audiodevicename.empty())
		{
			string audiodevicenamefound = "";
			std::map<int, string>::iterator it;
			std::map<int, string> devicemap;
			if ( (hostapitype < 0) && (ioflag == spiaudiodeviceINPUT))
			{
				devicemap = global_inputdevicemap;
			}
			else if ((hostapitype < 0) && (ioflag == spiaudiodeviceOUTPUT))
			{
				devicemap = global_outputdevicemap;
			}
			else if(hostapitype>-1)
			{ 
				if (hostapitype == paDirectSound)
				{
					devicemap = global_hostapimap_directsound;
				}
				else if (hostapitype == paMME)
				{
					devicemap = global_hostapimap_mme;
				}
				else if (hostapitype == paASIO)
				{
					devicemap = global_hostapimap_asio;
				}
				else if (hostapitype == paWDMKS)
				{
					devicemap = global_hostapimap_wdmks;
				}
				else if (hostapitype == paJACK)
				{
					devicemap = global_hostapimap_jack;
				}
				else if (hostapitype == paWASAPI)
				{
					devicemap = global_hostapimap_wasapi;
				}
				else
				{
					//error, but returns safely with default
					if (m_pFILE)
					{
						fprintf(m_pFILE, "error: hostapitype not specified correctly in SPIAudioDevice::MatchDevice(), will choose MME\n");
					}
					if (m_pFILE) fflush(m_pFILE);
					devicemap = global_hostapimap_mme;
				}
			}
			for (it = devicemap.begin(); it != devicemap.end(); it++)
			{
				string mystring = it->second;
				size_t found1 = audiodevicename.find(mystring);
				size_t found2 = mystring.find(audiodevicename);
				if ((found1 != std::string::npos) || (found2 != std::string::npos))
				{
					if (deviceidfound == -1)
					{
						//it->second    // string 
						audiodevicenamefound = it->second;
						//it->first   // value (key)
						deviceidfound = it->first;
						if (m_pFILE) fprintf(m_pFILE, "success: matching device found, %s will be retained.\n", audiodevicenamefound.c_str());
					}
					else
					{
						//error, but will return safely with first matching device
						if (m_pFILE) fprintf(m_pFILE, "warning: more than one matching device found, %s will be ignored.\n", mystring.c_str());
						break;
					}
					if (m_pFILE) fflush(m_pFILE);
				}
			}
			if (deviceidfound != -1) deviceid = deviceidfound;
		}
	}
	return deviceid;
}

//matchstring must be empty when called initially and non-empty when selecting device
//ioflag can be either 0 for all, 1 for input and 2 for output
int SPIAudioDevice::ScanAudioDevices(string matchmode/*=""*/, spiaudiodevicetypeflag ioflag/* =spiaudiodeviceALL*/)
{
	int deviceid = -2; //no match
	//if matchstring is empty, clear and repopulate all maps
	if (matchmode.empty())
	{
		global_alldevicemap.clear(); //all devicename deviceid map

		global_inputdevicemap.clear(); //inputdevicename deviceid map
		global_outputdevicemap.clear(); //outputdevicename deviceid map

		global_hostapimap.clear(); //hostapiname deviceid map

		global_hostapimap_hostapitype.clear(); //hostapiname hostapitype map 
		global_hostapimap_hostapitype.insert(pair<int, string>(paDirectSound, "DIRECTSOUND"));
		global_hostapimap_hostapitype.insert(pair<int, string>(paMME, "MME"));
		global_hostapimap_hostapitype.insert(pair<int, string>(paASIO, "ASIO"));
		global_hostapimap_hostapitype.insert(pair<int, string>(paWDMKS, "WDMKS"));
		global_hostapimap_hostapitype.insert(pair<int, string>(paWDMKS, "WDM-KS"));
		global_hostapimap_hostapitype.insert(pair<int, string>(paJACK, "JACK"));
		global_hostapimap_hostapitype.insert(pair<int, string>(paWASAPI, "WASAPI"));

		global_hostapimap_directsound.clear(); //directsound devicename deviceid map 
		global_hostapimap_mme.clear(); //mme devicename deviceid map 
		global_hostapimap_asio.clear(); //asio devicename deviceid map 
		global_hostapimap_wdmks.clear(); //wdmks devicename deviceid map 
		global_hostapimap_jack.clear(); //jack devicename deviceid map 
		global_hostapimap_wasapi.clear(); //wasapi devicename deviceid map 

		const PaDeviceInfo* deviceInfo;
		int numDevices = Pa_GetDeviceCount();
		for (int i = 0; i < numDevices; i++)
		{
			//devicenanme
			deviceInfo = Pa_GetDeviceInfo(i);
			string devicenamestring = deviceInfo->name;
			global_alldevicemap.insert(pair<int, string>(i, devicenamestring));
			if ((deviceInfo->maxInputChannels > 0) && ((ioflag==spiaudiodeviceALL)|| (ioflag==spiaudiodeviceINPUT))) global_inputdevicemap.insert(pair<int, string>(i, devicenamestring));
			  //else continue;
			if ((deviceInfo->maxOutputChannels > 0) && ((ioflag == spiaudiodeviceALL) || (ioflag == spiaudiodeviceOUTPUT))) global_outputdevicemap.insert(pair<int, string>(i, devicenamestring));
			  //else continue;
			//hostapi
			const PaHostApiInfo* hostInfo = Pa_GetHostApiInfo(deviceInfo->hostApi);
			string hostapistring = hostInfo->name;
			global_hostapimap.insert(pair<int, string>(i, hostapistring));
			//directsound
			if (hostInfo->type==paDirectSound)
			{
				global_hostapimap_directsound.insert(pair<int, string>(i, devicenamestring));
			}
			else if (hostInfo->type == paMME)
			{
				global_hostapimap_mme.insert(pair<int, string>(i, devicenamestring));
			}
			else if (hostInfo->type == paASIO)
			{
				global_hostapimap_asio.insert(pair<int, string>(i, devicenamestring));
			}
			else if (hostInfo->type == paWDMKS)
			{
				global_hostapimap_wdmks.insert(pair<int, string>(i, devicenamestring));
			}
			else if (hostInfo->type == paJACK)
			{
				global_hostapimap_jack.insert(pair<int, string>(i, devicenamestring));
			}
			else if (hostInfo->type == paWASAPI)
			{
				global_hostapimap_wasapi.insert(pair<int, string>(i, devicenamestring));
			}
			if (m_pFILE)
			{
				if (bdetailedreport)
				{
					fprintf(m_pFILE, "--------------------------------------- device #%d\n", i);
					//Mark global and API specific default devices
					int defaultDisplayed = 0;
					if (i == Pa_GetDefaultInputDevice())
					{
						fprintf(m_pFILE, "[ Default Input");
						defaultDisplayed = 1;
					}
					else if (i == Pa_GetHostApiInfo(deviceInfo->hostApi)->defaultInputDevice)
					{
						const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo(deviceInfo->hostApi);
						fprintf(m_pFILE, "[ Default %s Input", hostInfo->name);
						defaultDisplayed = 1;
					}

					if (i == Pa_GetDefaultOutputDevice())
					{
						fprintf(m_pFILE, (defaultDisplayed ? "," : "["));
						fprintf(m_pFILE, " Default Output");
						defaultDisplayed = 1;
					}
					else if (i == Pa_GetHostApiInfo(deviceInfo->hostApi)->defaultOutputDevice)
					{
						const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo(deviceInfo->hostApi);
						fprintf(m_pFILE, (defaultDisplayed ? "," : "["));
						fprintf(m_pFILE, " Default %s Output", hostInfo->name);
						defaultDisplayed = 1;
					}

					if (defaultDisplayed)
						fprintf(m_pFILE, " ]\n");

				}
				fprintf(m_pFILE, "id=%d, hostapi=%s, devicename=%s\n", i, hostapistring.c_str(), devicenamestring.c_str());
				if (bdetailedreport)
				{
					fprintf(m_pFILE, "Max inputs = %d", deviceInfo->maxInputChannels);
					fprintf(m_pFILE, ", Max outputs = %d\n", deviceInfo->maxOutputChannels);

					fprintf(m_pFILE, "Default low input latency   = %8.4f\n", deviceInfo->defaultLowInputLatency);
					fprintf(m_pFILE, "Default low output latency  = %8.4f\n", deviceInfo->defaultLowOutputLatency);
					fprintf(m_pFILE, "Default high input latency  = %8.4f\n", deviceInfo->defaultHighInputLatency);
					fprintf(m_pFILE, "Default high output latency = %8.4f\n", deviceInfo->defaultHighOutputLatency);
					//2022oct11, spi, begin
					//#ifdef WIN32
#if (defined(_WIN64) || defined(_WIN32))
//2022oct11, spi, end
#if PA_USE_ASIO
//ASIO specific latency information
					if (Pa_GetHostApiInfo(deviceInfo->hostApi)->type == paASIO) {
						long minLatency, maxLatency, preferredLatency, granularity;

						global_err = PaAsio_GetAvailableLatencyValues(i,
							&minLatency, &maxLatency, &preferredLatency, &granularity);

						fprintf(m_pFILE, "ASIO minimum buffer size    = %ld\n", minLatency);
						fprintf(m_pFILE, "ASIO maximum buffer size    = %ld\n", maxLatency);
						fprintf(m_pFILE, "ASIO preferred buffer size  = %ld\n", preferredLatency);

						if (granularity == -1)
							fprintf(m_pFILE, "ASIO buffer granularity     = power of 2\n");
						else
							fprintf(m_pFILE, "ASIO buffer granularity     = %ld\n", granularity);
					}
#endif //PA_USE_ASIO 
					//2022oct11, spi, begin
					//#endif //WIN32 
#endif //(defined(_WIN64) || defined(_WIN32))
//2022oct11, spi, end

				}
				fflush(m_pFILE);
			}
		}
		if (m_pFILE)
		{
			if (bdetailedreport)
			{
				fprintf(m_pFILE, "----------------------------------------------\n\n");
			}
			else
			{
				fprintf(m_pFILE, "\n\n");
			}
			fflush(m_pFILE);
		}
		return paNoDevice; //all good for initialization, no particular device should be returned
	}
	else
	{
		////////////////////////
		//matchmode=="loosely"
		////////////////////////
		//input device
		if (ioflag == spiaudiodeviceINPUT)
		{
			std::size_t found = global_audioinputdevicename.find(":");
			//if a semicolon is used to combined the hostapi name with the audio device name
			if (found != std::string::npos)
			{
				//host api may be: "Windows DirectSound","DirectSound", "Windows MME" "MME", "ASIO", "Windows WDM-KS", "Windows WDMKS", "WDM-KS", "WDMKS", "JACK", "Windows WASAPI", "WASAPI"
				global_audioinputhostapi = global_audioinputdevicename.substr(0, found);
				global_audioinputdevicename = global_audioinputdevicename.substr(found+1);
				if (m_pFILE)
				{
					fprintf(m_pFILE, "hostapi has been specified:\n");
					fprintf(m_pFILE, "global_audioinputhostapi=%s, global_audioinputdevicename=%s\n", global_audioinputhostapi.c_str(), global_audioinputdevicename.c_str());
				}
				if (m_pFILE) fflush(m_pFILE);
				//match hostapi
				int hostapitype = MatchHostAPI("loosely", spiaudiodeviceINPUT);
				//match device
				deviceid = MatchDevice("loosely", spiaudiodeviceINPUT, hostapitype);
			}
			else
			{
				if (m_pFILE)
				{
					fprintf(m_pFILE, "warning: hostapi has NOT been specified\n");
					fprintf(m_pFILE, "global_audioinputhostapi=%s, global_audioinputdevicename=%s\n", global_audioinputhostapi.c_str(), global_audioinputdevicename.c_str());
				}
				if (m_pFILE) fflush(m_pFILE);
				//match device
				deviceid = MatchDevice("loosely", spiaudiodeviceINPUT);
			}
		}
		//output device
		else if (ioflag == spiaudiodeviceOUTPUT)
		{
			std::size_t found = global_audiooutputdevicename.find(":");
			//if a semicolon is used to combined the hostapi name with the audio device name
			if (found != std::string::npos)
			{
				//host api may be: "Windows DirectSound","DirectSound", "Windows MME" "MME", "ASIO", "Windows WDM-KS", "Windows WDMKS", "WDM-KS", "WDMKS", "JACK", "Windows WASAPI", "WASAPI"
				global_audiooutputhostapi = global_audiooutputdevicename.substr(0, found);
				global_audiooutputdevicename = global_audiooutputdevicename.substr(found+1);
				if (m_pFILE)
				{
					fprintf(m_pFILE, "hostapi has been specified:\n");
					fprintf(m_pFILE, "global_audiooutputhostapi=%s, global_audiooutputdevicename=%s\n", global_audiooutputhostapi.c_str(), global_audiooutputdevicename.c_str());
				}
				if (m_pFILE) fflush(m_pFILE);
				//match hostapi
				int hostapitype = MatchHostAPI("loosely", spiaudiodeviceOUTPUT);
				//match device
				deviceid = MatchDevice("loosely", spiaudiodeviceOUTPUT, hostapitype);
			}
			else
			{
				
				if (m_pFILE)
				{
					if (!global_audiooutputdevicename.empty()) fprintf(m_pFILE, "warning: hostapi has NOT been specified:\n");
					fprintf(m_pFILE, "global_audiooutputhostapi=%s, global_audiooutputdevicename=%s\n", global_audiooutputhostapi.c_str(), global_audiooutputdevicename.c_str());
				}
				if (m_pFILE) fflush(m_pFILE);
				//match device
				deviceid = MatchDevice("loosely", spiaudiodeviceOUTPUT);
			}
		}
		else
		{
			return -2; //error for now
		}
	}
	return deviceid; //valid if > -1
}

bool SPIAudioDevice::SelectAudioInputDevice()
{
	//2021jan15, spi, begin
	if (global_inputdevicemap.empty())
	{
		int result = ScanAudioDevices(); //populate all devices
	}

	int deviceid = ScanAudioDevices("loosely", spiaudiodeviceINPUT);
	if (deviceid == paNoDevice) deviceid = Pa_GetDefaultInputDevice(); // default input device 
	//2021jan15, spi, end

	global_inputParameters.device = deviceid;
	global_inputParameters.channelCount = global_numchannels; // NUM_CHANNELS; //candidate
	global_inputParameters.sampleFormat = PA_SAMPLE_TYPE;
	global_inputParameters.suggestedLatency = Pa_GetDeviceInfo(global_inputParameters.device)->defaultLowOutputLatency;
	//Use an ASIO specific structure. WARNING - this is not portable. 
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



bool SPIAudioDevice::SelectAudioOutputDevice()
{
	//2021jan15, spi, begin
	if (global_outputdevicemap.empty())
	{
		int result = ScanAudioDevices(); //populate all devices
	}

	int deviceid = ScanAudioDevices("loosely", spiaudiodeviceOUTPUT);
	if (deviceid == paNoDevice)
	{
		//deviceid = Pa_GetDefaultOutputDevice(); // default output device 
		return false;
	}
	//2021jan15, spi, end

	global_outputParameters.device = deviceid;
	global_outputParameters.channelCount = global_numchannels; //NUM_CHANNELS;
	global_outputParameters.sampleFormat = PA_SAMPLE_TYPE;
	global_outputParameters.suggestedLatency = Pa_GetDeviceInfo(global_outputParameters.device)->defaultLowOutputLatency;
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

string SPIAudioDevice::GetHostAPIName(int deviceid)
{
	string hostapiname = "";
	if ((!global_hostapimap.empty()) && (deviceid>-1))
	{
		//global_hostapimap.find(deviceid);
		std::map<int, string>::iterator it;
		bool found = false;
		for (it = global_hostapimap.begin(); it != global_hostapimap.end(); it++)
		{
			//it->first    // value (key)
			//it->second   // string 
			if (it->first == deviceid)
			{
				found = true;
				hostapiname = it->second;
				break;
			}
		}
	}
	return hostapiname;
}

string SPIAudioDevice::GetDeviceName(int deviceid)
{
	string devicename = "";
	if ((!global_alldevicemap.empty()) && (deviceid>-1))
	{
		//global_devicemap.find(deviceid);
		std::map<int, string>::iterator it;
		bool found = false;
		for (it = global_alldevicemap.begin(); it != global_alldevicemap.end(); it++)
		{
			//it->first    // value (key)
			//it->second   // string 
			if (it->first == deviceid)
			{
				found = true;
				devicename = it->second;
				break;
			}
		}
	}
	return devicename;
}
