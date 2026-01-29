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
#ifndef _SPIUTILITY_H
#define _SPIUTILITY_H

//2021oct01, spi, begin
#define FFMPEG_APP_PATH "d:\\spibin\\ffmpeg\\ffmpeg.exe"
#define SPITEXT_EXEPATH	"D:\\spibin\\spitext\\Release\\spitext.exe"
//2021oct01, spi, end

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>

//2021nov16, spi, begin
#include <psapi.h> //for memory availability functions
#include <TCHAR.h> //for cpu availability functions
#include <pdh.h> //for cpu availability functions
#include <perflib.h> //for cpu availability functions
#include <winperf.h> //for cpu availability functions


#include <stdio.h>
#include <time.h> //for cross-platform current date time
#include <sstream>
//2021nov16, spi, end


using namespace std;


float RandomFloat(float a, float b);
int RandomInt(int lowest, int highest);

// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring &wstr);
// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str);


std::wstring s2ws(const std::string& str);
std::string ws2s(const std::wstring& wstr);

//2020july04, spi, begin
//RGB to HSV and HSV to RGB conversions from:
//https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
//note: code without floats (faster but less accurate):

typedef struct RgbColor
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	RgbColor()
	{
		RgbColor(0,0,0);
	}
	RgbColor(unsigned char red, unsigned char green, unsigned char blue)
	{
		r = red;
		g = green;
		b = blue;
	}
} RgbColor;

typedef struct HsvColor
{
	unsigned char h;
	unsigned char s;
	unsigned char v;
	HsvColor()
	{
		HsvColor(0,0,0);
	}
	HsvColor(unsigned char hue, unsigned char saturation, unsigned char value)
	{
		h = hue;
		s = saturation;
		v = value;
	}
} HsvColor;

RgbColor HsvToRgb(HsvColor hsv);
HsvColor RgbToHsv(RgbColor rgb);

#define UTILITY_HSVCOLORRANGE_TYPE_HUMANSKIN	0
#define UTILITY_HSVCOLORRANGE_TYPE_RED			1
#define UTILITY_HSVCOLORRANGE_TYPE_GREEN		2
#define UTILITY_HSVCOLORRANGE_TYPE_BLUE			3
#define UTILITY_HSVCOLORRANGE_TYPE_YELLOW		4
#define UTILITY_HSVCOLORRANGE_TYPE_ORANGE		5

bool IsInHSVColorRange(HsvColor myHsvColor, int hsvcolorrangetype);
//2020july04, spi, end


PCHAR*
CommandLineToArgvA(
PCHAR CmdLine,
int* _argc
);

//2021nov16, spi, begin
#define GETCURRENTDATETIME_YYYYMMDD_HHMMSS_DASHDOTSEMICOLON					0
#define GETCURRENTDATETIME_YYYYMMDD_HHMMSS_NODASHNODOTNOSEMICOLON			1
#define GETCURRENTDATETIME_YYYYMMDD_HHMMSS_SUBSEC_NODASHNODOTNOSEMICOLON	2
//get current date/time
//const std::string getcurrentdatetime(int stringformat = GETCURRENTDATETIME_YYYYMMDD_HHMMSS_SUBSEC_NODASHNODOTNOSEMICOLON);

#define SPILOGFILE_FILENAME	"log.txt"
class spilogfile
{
public:
	DWORD prev_nowstamp_ms;
	DWORD counter_subsec;
	/*
	int counter_submillisec;
	*/
public:
	ofstream ofs;
	spilogfile();
	spilogfile(string filename, bool timestamped = false);
	~spilogfile();
	bool init();
	bool write(string mystring, bool to_cout = false);
	bool writeline(string mystring, bool to_cout = false);
	const std::string getcurrentdatetime(int stringformat = GETCURRENTDATETIME_YYYYMMDD_HHMMSS_SUBSEC_NODASHNODOTNOSEMICOLON);
};
//2021nov16, spi, end





//2021nov29, spi, begin
#define SPISUBRECT_HORIZONTALFIRST	1
#define SPISUBRECT_VERTICALFIRST	0
#define SPISUBRECT_WRAPAROUND		1
#define SPISUBRECT_NOWRAPAROUND		0
#define SPISUBRECT_NOORPHANS		1
#define SPISUBRECT_ORPHANSALLOWED	0

#define SPISUBRECT_GRIDLIKE				1
#define SPISUBRECT_LISTLIKE_VERTICAL	2
#define SPISUBRECT_LISTLIKE_HORIZONTAL	4



//for submonitors and subvorogui
//divides referencerect by n same size rectangles (n is numberofsubrect)
//based on https://stackoverflow.com/questions/6190019/split-a-rectangle-into-equal-sized-rectangles
class spisubrect
{
public:
	vector<RECT> _rectvector;
	bool _horizontalfirst;
	bool _wraparound; //used by getnextrect() 
	int _columns;
	int _rows;
	int _fullrows;
	int _orphans;
	int _width;
	int _height;
	class spilogfile* p_spilogfile;

public:
	//note: if noorphans==true, numberofsubrect will be increased to fill the last row
	spisubrect(RECT referencerect, int numberofsubrect, bool horizontalfirst = true, bool wraparound = true, bool noorphans = false, int gridtypeflag=SPISUBRECT_GRIDLIKE);
	~spisubrect();
	int size();
	int cols();
	int rows();
	SIZE sizerect();
	RECT getrect(int index); //zero-based index
	POINT getrectcenter(int index); //zero-based index
	int getindex(int col, int row); //1-based col and row ids
	RECT getrect(int col, int row); //1-based col and row ids
	POINT getrectcenter(int col, int row); //1-based col and row ids
	int getindex(RECT fromrect);
	bool getcolrow(int index, int &col, int &row);
	RECT getnextrect(RECT fromrect);
	POINT getnextrectcenter(RECT fromrect);
};

#define MONITORRECTS_NUMBEROFSUBMONITORS_SHIFTDOW0N		4
#define MONITORRECTS_NUMBEROFSUBMONITORS_CONTROLDOWN	16
//2021nov29, spi, end


//2021nov29, spi, begin
//struct MonitorRects
class MonitorRects
//2021nov29, spi, end
{
public:
	std::vector<RECT>   rcMonitors;
	//2020sept13, spi, begin
	RECT rcCombined;
	//2020sept13, spi, end
	std::vector<HMONITOR>   hMonitors;
	std::map<HMONITOR, RECT> hmapMonitors;
	//std::map<RECT, HMONITOR> rcmapMonitors;
	//2021nov29, spi, begin
	std::vector<SIZE> monitorsizesvector; //unique monitor sizes, i.e. on remotedroide, spi's got two 4K monitors and one 2k monitor, this vector would contain 2 unique monitor sizes: 1 for the 2K and 1 for the 4K
	std::vector<SIZE> canvassizesvector; //unique canvas sizes, i.e. on remotedroide, this vector would contain 5 unique canvas sizes: 3840x2160, 1920x1080, 960x540, 480x270, and 240x135
	spilogfile* p_spilogfile;
	//2021nov29, spi, end
public:
	static BOOL CALLBACK MonitorEnum(HMONITOR hMon, HDC hdc, LPRECT lprcMonitor, LPARAM pData)
	{
		MonitorRects* pThis = reinterpret_cast<MonitorRects*>(pData);
		pThis->rcMonitors.push_back(*lprcMonitor);
		//2020sept13, spi, begin
		UnionRect(&pThis->rcCombined, &pThis->rcCombined, lprcMonitor);
		//2020sept13, spi, end
		pThis->hMonitors.push_back(hMon);
		pThis->hmapMonitors.insert(std::pair<HMONITOR, RECT>(hMon, *lprcMonitor));
		//pThis->rcmapMonitors.insert(std::pair<RECT, HMONITOR>(*lprcMonitor, hMon));
		RECT thismonitorrect = *lprcMonitor;
		if (pThis->CountMonitorRectSize(thismonitorrect) == 1)
		{
			//it's a new rect size
			SIZE monitorsize = { abs(thismonitorrect.right - thismonitorrect.left), abs(thismonitorrect.bottom - thismonitorrect.top) };
			pThis->monitorsizesvector.push_back(monitorsize);
			pThis->canvassizesvector.push_back(monitorsize);
			//
			spisubrect twobytwocanvas_spisubrect(thismonitorrect, 4, SPISUBRECT_HORIZONTALFIRST, SPISUBRECT_NOWRAPAROUND, SPISUBRECT_NOORPHANS);
			SIZE twobytwocanvas_size = twobytwocanvas_spisubrect.sizerect();
			if (pThis->CountCanvasSize(twobytwocanvas_size)==0)
			{
				//it's a new size
				pThis->canvassizesvector.push_back(twobytwocanvas_size);
			}
			spisubrect fourbyfourcanvas_spisubrect(thismonitorrect, 16, SPISUBRECT_HORIZONTALFIRST, SPISUBRECT_NOWRAPAROUND, SPISUBRECT_NOORPHANS);
			SIZE fourbyfourcanvas_size = fourbyfourcanvas_spisubrect.sizerect();
			if (pThis->CountCanvasSize(fourbyfourcanvas_size) == 0)
			{
				//it's a new size
				pThis->canvassizesvector.push_back(fourbyfourcanvas_size);
			}
			spisubrect eightbyeightcanvas_spisubrect(thismonitorrect, 64, SPISUBRECT_HORIZONTALFIRST, SPISUBRECT_NOWRAPAROUND, SPISUBRECT_NOORPHANS);
			SIZE eightbyeightcanvas_size = eightbyeightcanvas_spisubrect.sizerect();
			if (pThis->CountCanvasSize(eightbyeightcanvas_size) == 0)
			{
				//it's a new size
				pThis->canvassizesvector.push_back(eightbyeightcanvas_size);
			}
		}
		return TRUE;
	}

	MonitorRects()
	{
		//2020sept13, spi, begin
		SetRectEmpty(&rcCombined);
		//2020sept13, spi, end
		//2021nov29, spi, begin
		p_spilogfile = NULL;
		if(0) //if (SPIWINDOWTRANSPARENTMIDIVOROGUISPECTRUMTIMEFRAME_VS2017_MONITORRECTS_DEBUG)
		{
			p_spilogfile = DBG_NEW spilogfile("monitorrects_debug.txt", true);
		}
		//2021nov29, spi, end
		EnumDisplayMonitors(0, 0, MonitorEnum, (LPARAM)this);
	}

	//2021nov29, spi, begin
	~MonitorRects()
	{
		if (p_spilogfile) delete p_spilogfile;
	}

	RECT FindMonitorRect(HMONITOR hmon);
	RECT FindMonitorRect(HWND hwnd);

	RECT FindSubMonitorRect(HWND hwnd, int* p_index=NULL);

	RECT FindNextMonitorRect(HMONITOR hmon, HMONITOR* p_nexthmon=NULL);
	RECT FindPreviousMonitorRect(HMONITOR hmon, HMONITOR* p_previoushmon=NULL);
	RECT FindNextSubMonitorRect(HWND hwnd);
	RECT FindPreviousSubMonitorRect(HWND hwnd);

	int CountMonitorRectSize(RECT rect);
	int CountCanvasSize(SIZE size);
	int MatchCanvasSize(SIZE size);
	int MatchCanvasSize(LONG width, LONG height);
	//2021nov29, spi, end
};

struct EnumWindowsStruct
{
	std::map<HWND, wstring> hwndstringmap;
	wstring classnameprefix;

	static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
	{
		EnumWindowsStruct* pThis = reinterpret_cast<EnumWindowsStruct*>(lParam);
		WCHAR classname[1024];
		GetClassName(hwnd, classname, sizeof(classname));
		//if (wcsstr(classname, L"spiwindowtransparentclass") != NULL)
		if (wcsstr(classname, pThis->classnameprefix.c_str()) != NULL)
		{
			pThis->hwndstringmap.insert(pair<HWND, wstring>(hwnd, classname));
		}
		return TRUE;
	}

	EnumWindowsStruct()
	{
		ifstream myfile("classname.txt");
		string line;
		if (myfile.is_open())
		{
			getline(myfile, line);
			myfile.close();
			classnameprefix = utf8_decode(line);
		}
		else
		{
			classnameprefix = L"noclassnamespecified";
		}


		EnumWindows(EnumWindowsProc, (LPARAM)this);
	}

	void EnumWindowsStructRefresh()
	{
		hwndstringmap.clear();
		EnumWindows(EnumWindowsProc, (LPARAM)this);
	}
};

//alternate struct EnumWindowsStruct version as used in spitext.h/.cpp
struct EnumWindowsStruct_spitext
{
	std::map<HWND, wstring> hwndstringmap_class;
	wstring classnameprefix;
	std::map<HWND, wstring> hwndstringmap_title;

	static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
	{
		EnumWindowsStruct_spitext* pThis = reinterpret_cast<EnumWindowsStruct_spitext*>(lParam);
		WCHAR classname[1024];
		GetClassName(hwnd, classname, sizeof(classname));
		WCHAR windowtitle[1024];
		GetWindowText(hwnd, windowtitle, sizeof(windowtitle));
		if (pThis->classnameprefix.empty() || pThis->classnameprefix.c_str() == L"noclassnamespecified")
		{
			pThis->hwndstringmap_class.insert(pair<HWND, wstring>(hwnd, classname));
			pThis->hwndstringmap_title.insert(pair<HWND, wstring>(hwnd, windowtitle));
		}
		//if (wcsstr(classname, L"spiwindowtransparentclass") != NULL)
		else if (wcsstr(classname, pThis->classnameprefix.c_str()) != NULL)
		{
			pThis->hwndstringmap_class.insert(pair<HWND, wstring>(hwnd, classname));
			pThis->hwndstringmap_title.insert(pair<HWND, wstring>(hwnd, windowtitle));
		}
		return TRUE;
	}

	EnumWindowsStruct_spitext()
	{
		ifstream myfile("classname.txt");
		string line;
		if (myfile.is_open())
		{
			getline(myfile, line);
			myfile.close();
			if (!line.empty()) classnameprefix = utf8_decode(line);
		}
		else
		{
			classnameprefix = L"noclassnamespecified";
		}


		EnumWindows(EnumWindowsProc, (LPARAM)this);
	}

	void EnumWindowsStructRefresh()
	{
		hwndstringmap_class.clear();
		hwndstringmap_title.clear();
		EnumWindows(EnumWindowsProc, (LPARAM)this);
	}
};


//alternate struct EnumWindowsStruct version as used in spislideshowwin32.h/.cpp
#define SPISLIDESHOWWIN32_CLASSNAME_DEFAULT	L"spislideshowwin32class"	
#define SPISLIDESHOWWIN32_TITLE_DEFAULT	L"spislideshowwin32title"	
struct EnumWindowsStruct_spislideshowwin32
{
	std::map<HWND, wstring> hwndstringmap_class;
	wstring classnameprefix;
	std::map<HWND, wstring> hwndstringmap_title;

	static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
	{
		EnumWindowsStruct_spislideshowwin32* pThis = reinterpret_cast<EnumWindowsStruct_spislideshowwin32*>(lParam);
		WCHAR classname[1024];
		GetClassName(hwnd, classname, sizeof(classname));
		WCHAR windowtitle[1024];
		GetWindowText(hwnd, windowtitle, sizeof(windowtitle));
		if (pThis->classnameprefix.empty() || pThis->classnameprefix.c_str() == L"noclassnamespecified")
		{
			pThis->hwndstringmap_class.insert(pair<HWND, wstring>(hwnd, classname));
			pThis->hwndstringmap_title.insert(pair<HWND, wstring>(hwnd, windowtitle));
		}
		//if (wcsstr(classname, L"spiwindowtransparentclass") != NULL)
		else if (wcsstr(classname, pThis->classnameprefix.c_str()) != NULL)
		{
			pThis->hwndstringmap_class.insert(pair<HWND, wstring>(hwnd, classname));
			pThis->hwndstringmap_title.insert(pair<HWND, wstring>(hwnd, windowtitle));
		}
		return TRUE;
	}

	EnumWindowsStruct_spislideshowwin32()
	{
		classnameprefix = SPISLIDESHOWWIN32_CLASSNAME_DEFAULT;
		EnumWindows(EnumWindowsProc, (LPARAM)this);
	}

	void EnumWindowsStructRefresh()
	{
		hwndstringmap_class.clear();
		hwndstringmap_title.clear();
		EnumWindows(EnumWindowsProc, (LPARAM)this);
	}
};

//alternate struct EnumWindowsStruct version as used in spislideshowwin32.h/.cpp
#define SPISLIDESHOWWIN32_SPITEXT_CLASSNAME_DEFAULT	L"spislideshowwin32class_spitextclass"	
#define SPISLIDESHOWWIN32_SPITEXT_TITLE_DEFAULT	L"spislideshowwin32title_spitexttitle"	
struct EnumWindowsStruct_spislideshowwin32_spitext
{
	std::map<HWND, wstring> hwndstringmap_class;
	std::map<HWND, wstring> hwndstringmap_title;
	wstring classnameprefix;

	static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
	{
		EnumWindowsStruct_spislideshowwin32_spitext* pThis = reinterpret_cast<EnumWindowsStruct_spislideshowwin32_spitext*>(lParam);
		WCHAR classname[1024];
		GetClassName(hwnd, classname, sizeof(classname));
		WCHAR windowtitle[1024];
		GetWindowText(hwnd, windowtitle, sizeof(windowtitle));
		if (pThis->classnameprefix.empty() || pThis->classnameprefix.c_str() == L"noclassnamespecified")
		{
			pThis->hwndstringmap_class.insert(pair<HWND, wstring>(hwnd, classname));
			pThis->hwndstringmap_title.insert(pair<HWND, wstring>(hwnd, windowtitle));
		}
		//if (wcsstr(classname, L"spiwindowtransparentclass") != NULL)
		else if (wcsstr(classname, pThis->classnameprefix.c_str()) != NULL)
		{
			pThis->hwndstringmap_class.insert(pair<HWND, wstring>(hwnd, classname));
			pThis->hwndstringmap_title.insert(pair<HWND, wstring>(hwnd, windowtitle));
		}
		return TRUE;
	}

	EnumWindowsStruct_spislideshowwin32_spitext()
	{
		classnameprefix = SPISLIDESHOWWIN32_SPITEXT_CLASSNAME_DEFAULT;
		EnumWindows(EnumWindowsProc, (LPARAM)this);
	}

	void EnumWindowsStructRefresh()
	{
		hwndstringmap_class.clear();
		hwndstringmap_title.clear();
		EnumWindows(EnumWindowsProc, (LPARAM)this);
	}
};

std::string space2underscore(std::string text);
std::string dot2dash(std::string text);
std::string ampersand2underscore(std::string text);

bool copyFile(const char *SRC, const char* DEST); // copy in binary mode
bool hasEnding(std::string const &fullString, std::string const &ending);
string getPathName(const string& s);

int GetHammingDistance(const std::string &a, const std::string &b, const bool bfromend = false);
string getcommonprefix(const std::string &a, const std::string &b);
string getcommonsuffix(const std::string &a, const std::string &b);
void erasesubstr(std::string & mainStr, const std::string & toErase);


//2021oct15, spi, begin
string firstfolder(string path);
bool isdriveletter(string path);
string getdriveletter(string path); //without slash
bool haswildcard(string path);
string removesubstr(string path, string toremove);

bool isvalidfoldername(string path);
bool isvalidfilename(string path);
bool isvalidfilterstring(string path);

bool isvalidimagefilename(string path);
bool isvalidtxtfilename(string path);

bool isvalidtxtfilename_containingfoldernames(string path);
bool isvalidtxtfilename_containingfilenames(string path);
bool isvalidtxtfilename_containingfilterstrings(string path);

string getfolderfromfilename(string filename);
void removeduplicates(std::vector<string>& vec);
//2021oct15, spi, end


//2021nov16, spi, begin
//uses dir command to populate all found filenames into supplied txt file and supplied string vector
//searches for files matching supplied filter in supplied directory, including all subfolders by default
//returns files count
int getfilenamesfromfolder(std::string folderpath, std::string filefilter, std::string outputtxtfilename, vector<std::string> &outputstringvector, bool includesubfolders = true);
//2021nov16, spi, end


//2021nov16, spi, begin
//for windows and linux, see http link
//https://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process

DWORDLONG getmemory_totalavailable_physical();
DWORDLONG getmemory_totalavailable_virtual();
DWORDLONG getmemory_totalused_physical();
DWORDLONG getmemory_totalused_virtual();
SIZE_T getmemory_totalusedbyme_physical();
SIZE_T getmemory_totalusedbyme_virtual();

struct CPU_currentlyused
{
	//static PDH_HQUERY cpuQuery;
	PDH_HQUERY cpuQuery;
	//static PDH_HCOUNTER cpuTotal;
	PDH_HCOUNTER cpuTotal;

	CPU_currentlyused()
	{
		PdhOpenQuery(NULL, NULL, &cpuQuery);
		// You can also use L"\\Processor(*)\\% Processor Time" and get individual CPU values with PdhGetFormattedCounterArray()
		PdhAddEnglishCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
		PdhCollectQueryData(cpuQuery);
	}

	double getCurrentValue()
	{
		PDH_FMT_COUNTERVALUE counterVal;
		PdhCollectQueryData(cpuQuery);
		PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
		return counterVal.doubleValue;
	}
};

struct CPU_currentlyused_bycurrentprocess
{
	//static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
	ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
	//static int numProcessors;
	int numProcessors;
	//static HANDLE self;
	HANDLE self;

	CPU_currentlyused_bycurrentprocess()
	{
		SYSTEM_INFO sysInfo;
		FILETIME ftime, fsys, fuser;

		GetSystemInfo(&sysInfo);
		numProcessors = sysInfo.dwNumberOfProcessors;

		GetSystemTimeAsFileTime(&ftime);
		memcpy(&lastCPU, &ftime, sizeof(FILETIME));

		self = GetCurrentProcess();
		GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
		memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
		memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));
	}

	double getCurrentValue()
	{
		FILETIME ftime, fsys, fuser;
		ULARGE_INTEGER now, sys, user;
		double percent;

		GetSystemTimeAsFileTime(&ftime);
		memcpy(&now, &ftime, sizeof(FILETIME));

		GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
		memcpy(&sys, &fsys, sizeof(FILETIME));
		memcpy(&user, &fuser, sizeof(FILETIME));
		percent = (sys.QuadPart - lastSysCPU.QuadPart) +
			(user.QuadPart - lastUserCPU.QuadPart);
		percent /= (now.QuadPart - lastCPU.QuadPart);
		percent /= numProcessors;
		lastCPU = now;
		lastUserCPU = user;
		lastSysCPU = sys;

		return percent * 100;
	}
};



//2021nov16, spi, end

//2021dec29, spi, begin
bool bytebuffer_cell_setpixel(int icell, int jcell, BYTE* buffer, int width, int height, int numbyteperpixel, RGBQUAD pixelcolor = { 0,0,0,0 }, int cell_nx = 1, int cell_ny = 1,
	int pixel_inverseprobability = 1, int cell_inverseprobability = 1,
	RGBQUAD min_pixelcolor = { 0,0,0,0 }, RGBQUAD max_pixelcolor = { 255,255,255,0 }, int accelerationmode = 1);

bool bytebuffer_cellset_setpixel(BYTE* buffer, int width, int height, int numbyteperpixel, RGBQUAD pixelcolor={0,0,0,0}, int cell_nx=1, int cell_ny=1,
	int pixel_inverseprobability=1, int cell_inverseprobability=1,
	RGBQUAD min_pixelcolor={0,0,0,0}, RGBQUAD max_pixelcolor={255,255,255,0}, int accelerationmode=1);

bool bytebuffer_cellset_getsetpixel(BYTE* buffer_src, BYTE* buffer_dst, int width, int height, int numbyteperpixel, RGBQUAD pixelcolor = { 0,0,0,0 }, int cell_nx = 1, int cell_ny = 1,
	int pixel_inverseprobability = 1, int cell_inverseprobability = 1,
	RGBQUAD min_pixelcolor = { 0,0,0,0 }, RGBQUAD max_pixelcolor = { 255,255,255,0 }, int accelerationmode = 1,
	float fmixratio = 1.0, float fattenuation_b = 1.0, float fattenuation_g = 1.0, float fattenuation_r = 1.0);




#define SPITEXTCMDINEARGS_EXAMPLE_0	"\"SPI.RUN\" -1 3600 0 0 1920 \"Arial\" 1 \"spitextclass\" \"spitexttitle\" \"\" \"\" \"\" \"\" 100 \"2.1\" \"\" \"\" \"\" \"\" \"center\" \"center\" \"0.5\" \"0.5\" 1 "
#define SPITEXTCMDINEARGS_EXAMPLE_1	"\"COUNTDOWN HH:MM:SS\" -1 10 0 0 1920 \"Arial\" 1 \"spitextclass\" \"spitexttitle\" \"\" \"\" \"\" \"\" 100 \"2.1\" \"\" \"\" \"\" \"\" \"center\" \"center\" \"0.5\" \"0.5\" 1 "
#define SPITEXTCMDINEARGS_EXAMPLE_11	"\"COUNTDOWN HH:MM:SS\" -1 10 0 0 1920 \"Arial\" 1 \"spitextclass\" \"spitexttitle\" \"\" \"\" \"\" \"\" 100 \"1.1\" \"\" \"\" \"\" \"\" \"center\" \"center\" \"0.5\" \"0.5\" 1 "
#define SPITEXTCMDINEARGS_EXAMPLE_2	"\"CLOCK HH:MM:SS\" -1 60 0 0 1920 \"Arial\" 1 \"spitextclass\" \"spitexttitle\" \"\" \"\" \"\" \"\" 100 \"2.1\" \"\" \"\" \"\" \"\" \"center\" \"center\" \"0.5\" \"0.5\" 1 "

#define SPITEXTCMDLINEARGS_DEFAULTSTRING	" "

class spitextcommandlinearguments
{
public:
	string global_spitextstring = SPITEXTCMDLINEARGS_DEFAULTSTRING;
	int global_starttime_sec = -1; //user specified, -1 for not specified
	int global_endtime_sec = -1; //user specified, -1 for not specified
	int global_x = 0;
	int global_y = 0;
	int global_fontheight = 480;
	int global_fontwidth = -1; //will be computed within WM_PAINT handler
	string global_fontface = "Arial"; //see charmap.exe for fontface (Win+R>charmap)
	int global_idfontcolor = 0;
	TCHAR szTitle[1024] = { L"spitexttitle" };					// The title bar text
	TCHAR szWindowClass[1024] = { L"spitextclass" };			// the main window class name
	//string global_begin="begin.ahk";
	string global_begin = "";
	//string global_starting="starting.ahk";
	string global_starting = "";
	//string global_finishing="finishing.ahk";
	string global_finishing = "";
	//string global_end="end.ahk";
	string global_end = "";
	BYTE global_alpha = 220;
	//global_monitor is either empty "" (global_x and global_y will be considered absolute) 
	//or "1" or "2" or "2.1" or "2.2" or "2.3" or "2.4" specifying monitor (global_x and global_y will be considered relative)
	//global_monitor can also be using ":" instead of "." in which case there will be 16 posisble submonitors instead of 4, i.e. "1:1", "1:2", "1:3", ..., "1:15", "1:16", etc. for specifying monitor (global_x and global_y will be considered relative)
	string global_monitor = "";
	//global_hmonitor is either an empty string "" (global_x and global_y will be considered absolute) or the HMONITOR numeric value embeded into a string i.e. "0x00001E76" specifying a monitor handle (global_x and global_y will be considered relative)
	string global_hmonitor = "";
	//global_hwnd is either an empty string "" (global_x and global_y will be considered absolute) or the HWND numeric value embeded into a string i.e. "0x00001E76" specifying a window handle (global_x and global_y will be considered relative)
	string global_hwnd = "";
	//global_windowclass is either an empty string "" (global_x and global_y will be considered absolute) or the window class string i.e. "windowclassofinterest" (global_x and global_y will be considered relative)
	string global_windowclass = "";
	//global_windowtitle is either an empty string "" (global_x and global_y will be considered absolute) or the window title string i.e. "windowtitleofinterest" (global_x and global_y will be considered relative)
	string global_windowtitle = "";
	//global_horizontaljustification is either an empty string "" (global_x and global_y will be considered) or i.e. "left", "right", "center" (global_x and global_y will be ignored)
	string global_horizontaljustification = "";
	//global_verticaljustification is either an empty string "" (global_x and global_y will be considered) or i.e. "top", "bottom", "center" (global_x and global_y will be ignored)
	string global_verticaljustification = "";
	//global_horizontalforcefit is either 0 for no force fit or 1 for yes force fit (global_fontheight may be adjusted, only considered when global_x and global_y are relative to a monitor or to a window rect)
	//int global_horizontalforcefit = 0;
	float global_horizontalforcefit = 0.0f;
	//global_vecticalforcefit is either 0 for no force fit or 1 for yes force fit (global_fontheight may be adjusted, only considered when global_x and global_y are relative to a monitor or to a window rect)
	//int global_verticalforcefit = 0;
	float global_verticalforcefit = 0.0f;
	int global_fullmonitorssurface = 1; //global_fullmonitorssurface is either 0 for no or 1, the default, for yes do display string over multiple monitors

public:
	spitextcommandlinearguments(LPCSTR commandlinearguments/*=""*/)
	{
		setcommandlinestring(commandlinearguments);
	}

	~spitextcommandlinearguments()
	{
	}

	bool savebatchfile(string fullfilename, bool withenvironmentvariables=false, bool withexamples=false)
	{
		string quote = "\"";
		string space = " ";
		//create batch file content
		stringstream ss;
		stringstream ss2;
		if (withenvironmentvariables)
		{
			ss << "setlocal EnableDelayedExpansion" << endl;
			ss << endl;
			ss << "rem" << endl;
			ss << "rem echo off" << endl;
			if (withexamples)
			{
				ss << "rem global_spitextstring is the string to be displayed, it is also used to control clock and counter mode" << endl;
				ss << "rem set global_spitextstring = " << quote << "00000000" << quote << endl; 
				ss << "rem set global_spitextstring = " << quote << "12345678" << quote << endl; 
				ss << "rem set global_spitextstring = " << quote << "COUNTUP" << quote << endl; 
				ss << "rem set global_spitextstring = " << quote << "COUNTDOWN" << quote << endl; 
				ss << "rem set global_spitextstring = " << quote << "CLOCK" << quote << endl; 
				ss << "rem set global_spitextstring = " << quote << "COUNTUP HH:MM:SS" << quote << endl; 
				ss << "rem set global_spitextstring = " << quote << "COUNTDOWN HH:MM:SS" << quote << endl; 
				ss << "rem set global_spitextstring = " << quote << "CLOCK HH:MM:SS" << quote << endl; 
				ss << "rem set global_spitextstring = " << quote << "CLOCK HH:MM" << quote << endl; 
				ss << "rem set global_spitextstring = " << quote << "CLOCK MM:SS" << quote << endl; 
				ss << "rem set global_spitextstring = " << quote << "COUNTUP SS" << quote << endl; 
				ss << "rem set global_spitextstring = " << quote << "X" << quote << endl; 
				ss << "rem global_starttime_sec - 1 for not used" << endl;
				ss << "rem set /a global_starttime_sec = -1" << endl; 
				ss << "rem set /a global_starttime_sec = 0" << endl;
				ss << "rem set /a global_starttime_sec = 5" << endl;
				ss << "rem global_endtime_sec - 1 for not used" << endl;
				ss << "rem set /a global_endtime_sec = -1" << endl;
				ss << "rem set /a global_endtime_sec = 10" << endl;
				ss << "rem set /a global_endtime_sec = 5" << endl;
				ss << "rem set /a global_x = 0" << endl;
				ss << "rem set /a global_y = 0" << endl;
				ss << "rem set /a global_fontheight = 480" << endl;
				ss << "rem set /a global_fontheight = 960" << endl;
				ss << "rem set /a global_fontheight = 1920" << endl;
				ss << "rem i.e. " << quote << "Arial" << quote << ", etc." << endl;
				ss << "rem set global_fontface = " << quote << "Arial" << quote << endl;
				ss << "rem 0 for white, 1 for blue, 2 for red, 3 for black, 4 for yellow, 5 for green lime, 6 for green" << endl;
				ss << "rem set /a global_idfontcolor = 1" << endl;
				ss << "rem set szWindowClass = " << quote << "spitextclass" << quote << endl;
				ss << "rem set szTitle = " << quote << "spitexttitle" << quote << endl;
				ss << "rem set global_begin = " << quote << "begin.ahk" << quote << endl;
				ss << "rem set global_begin = " << quote << "" << quote << endl;
				ss << "rem set global_starting = " << quote << "starting.ahk" << quote << endl;
				ss << "rem set global_starting = " << quote << "" << quote << endl;
				ss << "rem set global_finishing = " << quote << "finishing.ahk" << quote << endl;
				ss << "rem set global_finishing = " << quote << "" << quote << endl;
				ss << "rem set global_end = " << quote << "end.ahk" << quote << endl;
				ss << "rem set global_end = " << quote << "" << quote << endl;
				ss << "rem global_alpha is the transparency, a value between 0 (invisible) and 255 (totaly visible, non - transparent)" << endl;
				ss << "rem set /a global_alpha = 200" << endl;
				ss << "rem set /a global_alpha = 150" << endl;
				ss << "rem set /a global_alpha = 100" << endl;
				ss << "rem global_monitor is either empty " << quote << quote << " (global_x and global_y will be considered absolute) or " << quote << "1" << quote << " or " << quote << "2" << quote << " or " << quote << "2.1" << quote << " or " << quote << "2.2" << quote << " or " << quote << "2.3" << quote << " or " << quote << "2.4" << quote << " specifying monitor(global_x and global_y will be considered relative)" << endl;
				ss << "rem global_monitor can also be using " << quote << ":" << quote << " instead of " << quote << "." << quote << " in which case there will be 16 posisble submonitors instead of 4, i.e. " << quote << "1:1" << quote << ", " << quote << "1:2" << quote << ", " << quote << "1:3" << quote << ", ..., " << quote << "1:15" << quote << ", "<< quote << "1:16" << quote << ", etc. for specifying monitor(global_x and global_y will be considered relative)" << endl;
				ss << "rem set global_monitor = """ << endl;
				ss << "rem set global_monitor = " << quote << "2.1" << quote << endl;
				ss << "rem set global_monitor = " << quote << "2.4" << quote << endl;
				ss << "rem set global_monitor = " << quote << "2.1" << quote << endl;
				ss << "rem set global_monitor = " << quote << "1:16" << quote << endl;
				ss << "rem set global_monitor = " << quote << "2:1" << quote << endl;
				ss << "rem set global_monitor = " << quote << "2:2" << quote << endl;
				ss << "rem set global_monitor = " << quote << "2:3" << quote << endl;
				ss << "rem set global_monitor = " << quote << "2:4" << quote << endl;
				ss << "rem set global_monitor = " << quote << "2:5" << quote << endl;
				ss << "rem set global_monitor = " << quote << "2:6" << quote << endl;
				ss << "rem set global_monitor = " << quote << "2:7" << quote << endl;
				ss << "rem set global_monitor = " << quote << "2:8" << quote << endl;
				ss << "rem set global_monitor = " << quote << "2:9" << quote << endl;
				ss << "rem set global_monitor = " << quote << "2:10" << quote << endl;
				ss << "rem set global_monitor = " << quote << "2:11" << quote << endl;
				ss << "rem set global_monitor = " << quote << "2:12" << quote << endl;
				ss << "rem set global_monitor = " << quote << "2:13" << quote << endl;
				ss << "rem set global_monitor = " << quote << "2:14" << quote << endl;
				ss << "rem set global_monitor = " << quote << "2:15" << quote << endl;
				ss << "rem set global_monitor = " << quote << "2:16" << quote << endl;
				ss << "rem global_hmonitor is either an empty string " << quote << quote <<" (global_x and global_y will be considered absolute) or the HMONITOR numeric value embeded into a string i.e. " << quote << "0x00001E76" << quote << " specifying a monitor handle(global_x and global_y will be considered relative)" << endl;
				ss << "rem set global_hmonitor = " << quote << quote << endl;
				ss << "rem set global_hmonitor = " << quote << "65537" << quote << endl;
				ss << "rem set global_hmonitor = " << quote << "0x00010001" << quote << endl;
				ss << "rem set global_hmonitor = " << quote << "65539" << quote << endl;
				ss << "rem set global_hmonitor = " << quote << "0x00010003" << quote << endl;
				ss << "rem global_hwnd is either an empty string " << quote << quote << " (global_x and global_y will be considered absolute) or the HWND numeric value embeded into a string i.e. " << quote << "0x00001E76" << quote << " specifying a window handle(global_x and global_y will be considered relative)" << endl;
				ss << "rem set global_hwnd = " << quote << quote << endl;
				ss << "rem set global_hwnd = " << quote << "0x000E0E0A" << quote << endl;
				ss << "rem set global_hwnd = " << quote << "0x00030512" << quote << endl;
				ss << "rem set global_hwnd = " << quote << "66262" << quote << endl;
				ss << "rem set global_hwnd = " << quote << "0x0002087C" << quote << endl;
				ss << "rem global_windowclass is either an empty string " << quote << quote <<" (global_x and global_y will be considered absolute) or the window class string i.e. " << quote << "windowclassofinterest" << quote << " (global_x and global_y will be considered relative)" << endl;
				ss << "rem set global_windowclass = " << quote << quote << endl;
				ss << "rem set global_windowclass = " << quote << "Chrome_WidgetWin_1" << quote << endl;
				ss << "rem global_windowtitle is either an empty string " << quote << quote << " (global_x and global_y will be considered absolute) or the window title string i.e. " << quote << "windowtitleofinterest" << quote << " (global_x and global_y will be considered relative)" << endl;
				ss << "rem set global_windowtitle = " << quote << quote << endl;
				ss << "rem set global_windowtitle = " << quote << "fprintf - C++ Reference - Google Chrome" << quote << endl;
				ss << "rem global_horizontaljustification is either an empty string "" (global_x and global_y will be considered) or i.e. " << quote << "left" << quote << ", " << quote << "right" << quote << ", " << quote << "center" << quote << " (global_x and global_y will be ignored)" << endl;
				ss << "rem set global_horizontaljustification = " << quote << quote << endl;
				ss << "rem set global_horizontaljustification = " << quote << "center" << quote << endl;
				ss << "rem set global_horizontaljustification = " << quote << "right" << quote << endl;
				ss << "rem global_verticaljustification is either an empty string " << quote << quote << " (global_x and global_y will be considered) or i.e. " << quote << "top" << quote << ", " << quote << "bottom" << quote << ", " << quote << "center" << quote << " (global_x and global_y will be ignored)" << endl;
				ss << "rem set global_verticaljustification = " << quote << quote << endl;
				ss << "rem set global_verticaljustification = " << quote << "center" << quote << endl;
				ss << "rem set global_verticaljustification = " << quote << "bottom" << quote << endl;
				ss << "rem global_horizontalforcefit is either 0 for no force fit or 1 for yes force fit(global_fontheight may be adjusted, only considered when global_x and global_y are relative to a monitor or to a window rect)" << endl;
				ss << "rem set global_horizontalforcefit = " << quote << "0.0" << quote << endl;
				ss << "rem set global_horizontalforcefit = " << quote << "0.5" << quote << endl;
				ss << "rem global_verticalforcefit is either 0 for no force fit or 1 for yes force fit(global_fontheight may be adjusted, only considered when global_x and global_y are relative to a monitor or to a window rect)" << endl;
				ss << "rem set global_verticalforcefit = " << quote << "0.0" << quote << endl;
				ss << "rem set global_verticalforcefit = " << quote << "0.5" << quote << endl;
				ss << "rem global_fullmonitorssurface is either 0 for no or 1, the default, for yes do display string over multiple monitors" << endl;
				ss << "rem set /a global_fullmonitorssurface = 1" << endl;
			}
			ss << "set global_spitextstring = " << quote << global_spitextstring << quote << endl;
			ss << "set /a global_starttime_sec = " << to_string(global_starttime_sec) << endl;
			ss << "set /a global_endtime_sec = " << to_string(global_endtime_sec) << endl;
			ss << "set /a global_x = " << to_string(global_x) << endl;
			ss << "set /a global_y = " << to_string(global_y) << endl;
			ss << "set /a global_fontheight = " << to_string(global_fontheight) << endl;
			ss << "set /a global_fontwidth = " << to_string(global_fontwidth) << endl;
			ss << "set global_fontface = " << quote << global_fontface << quote << endl;
			ss << "set /a global_idfontcolor = " << to_string(global_idfontcolor) << endl;
			ss << "set szWindowClass = " << quote << getspitextwindowclass() << quote << endl;
			ss << "set szTitle = " << quote << getspitexttitle() << quote << endl;
			ss << "set global_begin = " << quote << global_begin << quote << endl;
			ss << "set global_starting = " << quote << global_starting << quote << endl;
			ss << "set global_finishing = " << quote << global_finishing << quote << endl;
			ss << "set global_end = " << quote << global_end << quote << endl;
			ss << "set /a global_alpha = " << to_string(global_alpha) << endl;
			ss << "set global_spitextstring = " << quote << global_monitor << quote << endl;
			ss << "set global_spitextstring = " << quote << global_hmonitor << quote << endl;
			ss << "set global_spitextstring = " << quote << global_hwnd << quote << endl;
			ss << "set global_spitextstring = " << quote << global_windowclass << quote << endl;
			ss << "set global_spitextstring = " << quote << global_windowtitle << quote << endl;
			ss << "set global_spitextstring = " << quote << global_horizontaljustification << quote << endl;
			ss << "set global_spitextstring = " << quote << global_verticaljustification << quote << endl;
			ss << "set /a global_spitextstring = " << to_string(global_horizontalforcefit) << endl;
			ss << "set /a global_spitextstring = " << to_string(global_verticalforcefit) << endl;
			ss << "set /a global_spitextstring = " << to_string(global_fullmonitorssurface) << endl;
			ss << "rem execute spitext.exe" << endl;
			ss2 << quote << quote << space << quote << "spitext.exe" << quote << space << "%global_spitextstring% %global_starttime_sec% %global_endtime_sec% %global_x% %global_y% %global_fontheight% %global_fontface% %global_idfontcolor% %szWindowClass% %szTitle% %global_begin% %global_starting% %global_finishing% %global_end% %global_alpha% %global_monitor% %global_hmonitor% %global_hwnd% %global_windowclass% %global_windowtitle% %global_horizontaljustification% %global_verticaljustification% %global_horizontalforcefit% %global_verticalforcefit% %global_fullmonitorssurface%";
			ss << "start " << ss2.str() << endl;
			ss << "echo " << ss2.str() << space << "> " << quote << fullfilename << ".txt" << quote << endl;	
		}
		else
		{
			bool withspitextexeprefix = true;
			string cmdline = getcommandlinestring(withspitextexeprefix);
			stringstream ss2;
			ss << "start " << quote << quote << space << cmdline << endl;
		}
		//save batch file
		ofstream ofs;
		ofs.open(fullfilename);
		ofs << ss.str();
		ofs.close();
		return true;
	}

	string getcommandlinestring(bool withspitextexeprefix=false)
	{
		string quote = "\"";
		string space = " ";
		string commandlinestring = "";
		if (withspitextexeprefix) commandlinestring += quote + "spitext.exe" + quote + space;
		commandlinestring += quote + global_spitextstring + quote + space;
		commandlinestring += to_string(global_starttime_sec) + space;
		commandlinestring += to_string(global_endtime_sec) + space;
		commandlinestring += to_string(global_x) + space;
		commandlinestring += to_string(global_y) + space;
		commandlinestring += to_string(global_fontheight) + space;
		commandlinestring += to_string(global_fontwidth) + space;
		commandlinestring += quote + global_fontface + quote + space;
		commandlinestring += to_string(global_idfontcolor) + space;
		commandlinestring += quote + getspitextwindowclass() + quote + space;
		commandlinestring += quote + getspitexttitle() + quote + space;
		commandlinestring += quote + global_begin + quote + space;
		commandlinestring += quote + global_starting + quote + space;
		commandlinestring += quote + global_finishing + quote + space;
		commandlinestring += quote + global_end + quote + space;
		commandlinestring += to_string(global_alpha) + space;
		commandlinestring += quote + global_monitor + quote + space;
		commandlinestring += quote + global_hmonitor + quote + space;
		commandlinestring += quote + global_hwnd + quote + space;
		commandlinestring += quote + global_windowclass + quote + space;
		commandlinestring += quote + global_windowtitle + quote + space;
		commandlinestring += quote + global_horizontaljustification + quote + space;
		commandlinestring += quote + global_verticaljustification + quote + space;
		commandlinestring += to_string(global_horizontalforcefit) + space;
		commandlinestring += to_string(global_verticalforcefit) + space;
		commandlinestring += to_string(global_fullmonitorssurface); // +space;

		return commandlinestring;
	}

	bool setcommandlinestring(string s="")
	{
		return setcommandlinestring(s.c_str());
	}

	bool setcommandlinestring(LPCSTR commandlinearguments="")
	{
		string commandlinestring(commandlinearguments);
		wstring commandlinewstring;
		if (commandlinestring.empty())
		{
			commandlinestring = SPITEXTCMDINEARGS_EXAMPLE_0;
		}
		commandlinestring = "spitext.exe " + commandlinestring;
		commandlinewstring = utf8_decode(commandlinestring);

		LPSTR *szArgList;
		int nArgs;
		szArgList = CommandLineToArgvA((PCHAR)commandlinestring.c_str(), &nArgs);//CommandLineToArgvA(GetCommandLineA(), &nArgs);
		if (NULL == szArgList)
		{
			//wprintf(L"CommandLineToArgvA failed\n");
			return false;
		}
		LPWSTR *szArgListW;
		int nArgsW;
		szArgListW = CommandLineToArgvW(commandlinewstring.c_str(), &nArgsW); //CommandLineToArgvW(GetCommandLineW(), &nArgsW);
		if (NULL == szArgListW)
		{
			//wprintf(L"CommandLineToArgvW failed\n");
			return false;
		}
		if (nArgs > 1)
		{
			global_spitextstring = szArgList[1]; //any text to be displayed or "COUNTUP", "COUNTDOWN", "CLOCK", or a combinaison of "COUNTUP", "COUNTDOWN", "CLOCK" along with "HH", "MM", "SS", 
												 //"HH:MM:SS", "HH:MM", "MM:SS", "COUNTUP HH:MM:SS", "COUNTDOWN MM:SS", "CLOCK HH:MM:SS"
		}
		if (nArgs > 2)
		{
			global_starttime_sec = atoi(szArgList[2]); //-1, 0 or positive integer specifying seconds, used to setup display string when global_spitextstring contains "COUNTUP", "COUNTDOWN", "CLOCK" 
														//otherwise with any other global_spitextstring, global_starttime_sec is used as a delay before initiating global_spitextstring display
		}
		if (nArgs > 3)
		{
			global_endtime_sec = atoi(szArgList[3]); //-1, 0 or positive integer specifying seconds, used to setup display string when global_spitextstring contains "COUNTUP", "COUNTDOWN", "CLOCK"
													//otherwise with any other global_spitextstring, global_starttime_sec is used as a delay before killing global_spitextstring display
		}
		if (nArgs > 4)
		{
			global_x = atoi(szArgList[4]);
		}
		if (nArgs > 5)
		{
			global_y = atoi(szArgList[5]);
		}
		if (nArgs > 6)
		{
			global_fontheight = atoi(szArgList[6]);
		}
		//new parameters
		if (nArgs > 7)
		{
			global_fontface = szArgList[7];
		}
		if (nArgs > 8)
		{
			global_idfontcolor = atoi(szArgList[8]);
		}
		if (nArgs > 9)
		{
			wcscpy(szWindowClass, szArgListW[9]);
		}
		if (nArgs > 10)
		{
			wcscpy(szTitle, szArgListW[10]);
		}
		if (nArgs > 11)
		{
			global_begin = szArgList[11];
		}
		if (nArgs > 12)
		{
			global_starting = szArgList[12];
		}
		if (nArgs > 13)
		{
			global_finishing = szArgList[13];
		}
		if (nArgs > 14)
		{
			global_end = szArgList[14];
		}
		if (nArgs > 15)
		{
			global_alpha = atoi(szArgList[15]);
		}
		if (nArgs > 16)
		{
			global_monitor = szArgList[16]; //global_monitor is either empty "" (global_x and global_y will be considered absolute) or i.e. "1" or "2" or "2.1" or "2.2" or "2.3" or "2.4" etc. specifying monitor (global_x and global_y will be considered relative)
											//global_monitor can also be using ":" instead of "." in which case there will be 16 posisble submonitors instead of 4, i.e. "1:1", "1:2", "1:3", ..., "1:15", "1:16", etc. for specifying monitor (global_x and global_y will be considered relative)
		}
		if (nArgs > 17)
		{
			global_hmonitor = szArgList[17]; //global_hmonitor is either an empty string "" (global_x and global_y will be considered absolute) or the HMONITOR numeric value embeded into a string i.e. "0x00001E76" specifying a monitor handle (global_x and global_y will be considered relative)
		}
		if (nArgs > 18)
		{
			global_hwnd = szArgList[18]; //global_hwnd is either an empty string "" (global_x and global_y will be considered absolute) or the HWND numeric value embeded into a string i.e. "0x00001E76" specifying a window handle (global_x and global_y will be considered relative)
		}
		if (nArgs > 19)
		{
			global_windowclass = szArgList[19]; //global_windowclass is either an empty string "" (global_x and global_y will be considered absolute) or the window class string i.e. "windowclassofinterest" (global_x and global_y will be considered relative)
		}
		if (nArgs > 20)
		{
			global_windowtitle = szArgList[20]; //global_windowtitle is either an empty string "" (global_x and global_y will be considered absolute) or the window title string i.e. "windowtitleofinterest" (global_x and global_y will be considered relative)
		}
		if (nArgs > 21)
		{
			global_horizontaljustification = szArgList[21]; //global_horizontaljustification is either an empty string "" (global_x and global_y will be considered) or i.e. "left", "right", "center" (global_x and global_y will be ignored)
		}
		if (nArgs > 22)
		{
			global_verticaljustification = szArgList[22]; //global_verticaljustification is either an empty string "" (global_y and global_y will be considered) or i.e. "top", "bottom", "center" (global_x and global_y will be ignored)
		}
		if (nArgs > 23)
		{
			//global_horizontalforcefit = atoi(szArgList[23]); //global_horizontalforcefit is either 0 for no force fit or 1 for yes force fit (global_fontheight may be adjusted, only considered when global_x and global_y are relative to a monitor or to a window rect)
			global_horizontalforcefit = atof(szArgList[23]); //global_horizontalforcefit is either 0 for no force fit or 1 for yes force fit (global_fontheight may be adjusted, only considered when global_x and global_y are relative to a monitor or to a window rect)
			if (global_horizontalforcefit < 0.0f) global_horizontalforcefit = 0.0f;
			if (global_horizontalforcefit > 1.0f) global_horizontalforcefit = 1.0f;
		}
		if (nArgs > 24)
		{
			//global_vecticalforcefit = atoi(szArgList[24]); //global_vecticalforcefit is either 0 for no force fit or 1 for yes force fit (global_fontheight may be adjusted, only considered when global_x and global_y are relative to a monitor or to a window rect)
			global_verticalforcefit = atof(szArgList[24]); //global_vecticalforcefit is either 0 for no force fit or 1 for yes force fit (global_fontheight may be adjusted, only considered when global_x and global_y are relative to a monitor or to a window rect)
			if (global_verticalforcefit < 0.0f) global_verticalforcefit = 0.0f;
			if (global_verticalforcefit > 1.0f) global_verticalforcefit = 1.0f;
		}
		if (nArgs > 25)
		{
			global_fullmonitorssurface = atoi(szArgList[25]); //global_fullmonitorssurface is either 0 for no or 1, the default, for yes do display string over multiple monitors
		}
		LocalFree(szArgList);
		LocalFree(szArgListW);
		return true;
	}

	string getspitextstring()
	{
		return global_spitextstring;
	}
	bool setspitextstring(string s)
	{
		global_spitextstring = s;
		return true;
	}
	int getstarttime_sec()
	{
		return global_starttime_sec;
	}
	bool setstarttime_sec(int value)
	{
		global_starttime_sec = value;
		return true;
	}
	int getendtime_sec()
	{
		return global_endtime_sec;
	}
	bool setendtime_sec(int value)
	{
		global_endtime_sec = value;
		return true;
	}
	int getx()
	{
		return global_x;
	}
	bool setx(int value)
	{
		global_x = value;
		return true;
	}
	int gety()
	{
		return global_y;
	}
	bool sety(int value)
	{
		global_y = value;
		return true;
	}
	int getfontheight()
	{
		return global_fontheight;
	}
	bool setfontheight(int value)
	{
		global_fontheight = value;
		return true;
	}
	int getfontwidth()
	{
		return global_fontwidth;
	}
	bool setfontwidth(int value)
	{
		global_fontwidth = value;
		return true;
	}

	string getfontface()
	{
		return global_fontface;
	}
	bool setfontface(string s)
	{
		global_fontface = s;
		return true;
	}

	int getidfontcolor()
	{
		return global_idfontcolor;
	}
	bool setidfontcolor(int value)
	{
		global_idfontcolor = value;
		return true;
	}

	string getspitexttitle()
	{
		string title = utf8_encode(getspitexttitle_w());
		return title;
	}
	bool setspitexttitle(string s)
	{
		wstring ws = utf8_decode(s);
		setspitexttitle_w(ws);
		return true;
	}
	wstring getspitexttitle_w()
	{
		wstring ws(szTitle);
		return ws;
	}
	bool setspitexttitle_w(wstring ws)
	{
		//wcscpy(szTitle, szArgListW[9]);
		wcscpy(szTitle, ws.c_str());
		return true;
	}

	string getspitextwindowclass()
	{
		string windowclass = utf8_encode(getspitextwindowclass_w());
		return windowclass;
	}
	bool setspitextwindowclass(string s)
	{
		wstring ws = utf8_decode(s);
		setspitextwindowclass_w(ws);
		return true;
	}

	wstring getspitextwindowclass_w()
	{
		wstring ws(szWindowClass);
		return ws;
	}
	bool setspitextwindowclass_w(wstring ws)
	{
		wcscpy(szWindowClass, ws.c_str());
		return true;
	}

	string getbegin()
	{
		return global_begin;
	}
	bool setbegin(string s)
	{
		global_begin = s;
		return true;
	}

	string getstarting()
	{
		return global_starting;
	}
	bool setstarting(string s)
	{
		global_starting = s;
		return true;
	}

	string getfinishing()
	{
		return global_finishing;
	}
	bool setfinishing(string s)
	{
		global_finishing = s;
		return true;
	}

	string getend()
	{
		return global_end;
	}
	bool setend(string s)
	{
		global_end = s;
		return true;
	}

	BYTE getalpha()
	{
		return global_alpha;
	}
	bool setalpha(BYTE value)
	{
		global_alpha = value;
		return true;
	}

	string getmonitor()
	{
		return global_monitor;
	}
	bool setmonitor(string s)
	{
		global_monitor = s;
		return true;
	}

	string gethmonitor()
	{
		return global_hmonitor;
	}
	bool sethmonitor(string s)
	{
		global_hmonitor = s;
		return true;
	}


	string gethwnd()
	{
		return global_hwnd;
	}
	bool sethwnd(string s)
	{
		global_hwnd = s;
		return true;
	}

	string getwindowclass()
	{
		return global_windowclass;
	}
	bool setwindowclass(string s)
	{
		global_windowclass = s;
		return true;
	}

	string getwindowtitle()
	{
		return global_windowtitle;
	}
	bool setwindowtitle(string s)
	{
		global_windowtitle = s;
		return true;
	}

	string gethorizontaljustification()
	{
		return global_horizontaljustification;
	}
	bool sethorizontaljustification(string s)
	{
		global_horizontaljustification = s;
		return true;
	}

	string getverticaljustification()
	{
		return global_verticaljustification;
	}
	bool setverticaljustification(string s)
	{
		global_verticaljustification = s;
		return true;
	}


	float gethorizontalforcefit()
	{
		return global_horizontalforcefit;
	}
	bool sethorizontalforcefit(float value)
	{
		global_horizontalforcefit = value;
		return true;
	}

	float getverticalforcefit()
	{
		return global_verticalforcefit;
	}
	bool setverticalforcefit(float value)
	{
		global_verticalforcefit = value;
		return true;
	}

	int getfullmonitorssurface()
	{
		return global_fullmonitorssurface;
	}
	bool setfullmonitorssurface(int value)
	{
		global_fullmonitorssurface = value;
		return true;
	}

};
//2021dec29, spi, end



#endif
