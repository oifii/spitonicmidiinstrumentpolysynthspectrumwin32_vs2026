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

//#pragma once
#ifndef _SPIAUDIODEVICE_H
#define _SPIAUDIODEVICE_H

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

using namespace std;

typedef enum spiaudiodevicetypeflag
{
	spiaudiodeviceALL = 0,
	spiaudiodeviceINPUT = 1,
	spiaudiodeviceOUTPUT = 2
} spiaudiodevicetypeflag;

class SPIAudioDevice
{
public:
	std::map<int, string> global_alldevicemap;
	std::map<int, string> global_inputdevicemap;
	std::map<int, string> global_outputdevicemap;
	std::map<int, string> global_hostapimap;
	std::map<int, string> global_hostapimap_hostapitype;
	std::map<int, string> global_hostapimap_directsound;
	std::map<int, string> global_hostapimap_mme;
	std::map<int, string> global_hostapimap_asio;
	std::map<int, string> global_hostapimap_wdmks;
	std::map<int, string> global_hostapimap_jack;
	std::map<int, string> global_hostapimap_wasapi;

	PaStream* global_stream;
	PaStreamParameters global_inputParameters;
	PaStreamParameters global_outputParameters;
	PaError global_err;
	string global_audioinputdevicename; // = "";
	string global_audiooutputdevicename; // = "";

	string global_audioinputhostapi; // = "";
	string global_audiooutputhostapi; // = "";

	int global_inputAudioChannelSelectors[2];
	int global_outputAudioChannelSelectors[2];
	PaAsioStreamInfo global_asioInputInfo;
	PaAsioStreamInfo global_asioOutputInfo;

	//2022oct11, spi, begin
	bool bdetailedreport;
	//2022oct11, spi, end
	FILE* m_pFILE;// = NULL;

public:
	SPIAudioDevice();
	~SPIAudioDevice();

	int ScanAudioDevices(string matchmode = "", spiaudiodevicetypeflag ioflag=spiaudiodeviceALL);
	int MatchHostAPI(string matchmode = "", spiaudiodevicetypeflag ioflag=spiaudiodeviceALL);
	int MatchDevice(string matchmode = "", spiaudiodevicetypeflag ioflag = spiaudiodeviceALL, int hostapitype=-1);

	bool SelectAudioInputDevice();
	bool SelectAudioOutputDevice();

	string GetHostAPIName(int deviceid);
	string GetDeviceName(int deviceid);
};


#endif //_SPIAUDIODEVICE_H
