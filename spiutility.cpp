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

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
 //#define new DBG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#include <stdlib.h>     // srand, rand 
#include <Windows.h>

//#include <stdlib.h>     // srand, rand 
#include <string>
#include <shellapi.h> //for ShellExecute()

#include <codecvt>

//#include "spiutility.h"
#include "spiregreadwrite.h"

#include "spitonicmidiinstrumentsynthwin32.h" //spiutility.h now needs this header to be defined before
#include "spiutility.h"


float RandomFloat(float a, float b)
{
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

int RandomInt(int lowest, int highest)
{
	int range = (highest - lowest) + 1;
	int random_integer = lowest + int(range*rand() / (RAND_MAX + 1.0));
	return random_integer;
}



// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring &wstr)
{
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str)
{
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

std::wstring s2ws(const std::string& str)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	//using convert_typeX = std::codecvt_utf16<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.from_bytes(str);
}

std::string ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	//using convert_typeX = std::codecvt_utf16<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}


//2020july04, spi, begin
//RGB to HSV and HSV to RGB conversions from:
//https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
//note: code without floats (faster but less accurate):

/* //defined in utility.h
typedef struct RgbColor
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
} RgbColor;

typedef struct HsvColor
{
	unsigned char h;
	unsigned char s;
	unsigned char v;
} HsvColor;
*/

RgbColor HsvToRgb(HsvColor hsv)
{
	RgbColor rgb;
	unsigned char region, remainder, p, q, t;

	if (hsv.s == 0)
	{
		rgb.r = hsv.v;
		rgb.g = hsv.v;
		rgb.b = hsv.v;
		return rgb;
	}

	region = hsv.h / 43;
	remainder = (hsv.h - (region * 43)) * 6;

	p = (hsv.v * (255 - hsv.s)) >> 8;
	q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
	t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

	switch (region)
	{
	case 0:
		rgb.r = hsv.v; rgb.g = t; rgb.b = p;
		break;
	case 1:
		rgb.r = q; rgb.g = hsv.v; rgb.b = p;
		break;
	case 2:
		rgb.r = p; rgb.g = hsv.v; rgb.b = t;
		break;
	case 3:
		rgb.r = p; rgb.g = q; rgb.b = hsv.v;
		break;
	case 4:
		rgb.r = t; rgb.g = p; rgb.b = hsv.v;
		break;
	default:
		rgb.r = hsv.v; rgb.g = p; rgb.b = q;
		break;
	}

	return rgb;
}

HsvColor RgbToHsv(RgbColor rgb)
{
	HsvColor hsv;
	unsigned char rgbMin, rgbMax;

	rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
	rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);

	hsv.v = rgbMax;
	if (hsv.v == 0)
	{
		hsv.h = 0;
		hsv.s = 0;
		return hsv;
	}

	hsv.s = 255 * long(rgbMax - rgbMin) / hsv.v;
	if (hsv.s == 0)
	{
		hsv.h = 0;
		return hsv;
	}

	if (rgbMax == rgb.r)
		hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
	else if (rgbMax == rgb.g)
		hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
	else
		hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);

	return hsv;
}

bool IsInHSVColorRange(HsvColor myHsvColor, int hsvcolorrangetype)
{
	//UTILITY_HSVCOLORRANGE_TYPE_HUMANSKIN	
	//UTILITY_HSVCOLORRANGE_TYPE_RED			
	//UTILITY_HSVCOLORRANGE_TYPE_BLUE			
	//UTILITY_HSVCOLORRANGE_TYPE_YELLOW
	if (hsvcolorrangetype == UTILITY_HSVCOLORRANGE_TYPE_HUMANSKIN)
	{
		/*
		//# define the upper and lower boundaries of the HSV pixel
		//# intensities to be considered 'skin'
		//lower = np.array([0, 48, 80], dtype = "uint8")
		//upper = np.array([20, 255, 255], dtype = "uint8")
		if ( (myHsvColor.h >=0) && (myHsvColor.h <= 20) )
		{
			if ((myHsvColor.s >= 48) && (myHsvColor.s <= 255))
			{
				if ((myHsvColor.v >= 80) && (myHsvColor.v <= 255))
				{
					//if 3 conditions are meet, hsv color is in range
					return true;
				}
			}
		}
		*/
		//alternatively, from:
		//https://stackoverflow.com/questions/8753833/exact-skin-color-hsv-range#:~:text=The%20skin%20in%20channel%20H,for%20small%20values%20of%20V.
		//lower_color = np.array([108, 23, 82])
		//upper_color = np.array([179, 255, 255])
		/*
		if ((myHsvColor.h >= 108) && (myHsvColor.h <= 179))
		{
			if ((myHsvColor.s >= 23) && (myHsvColor.s <= 255))
			{
				if ((myHsvColor.v >= 82) && (myHsvColor.v <= 255))
				{
					//if 3 conditions are meet, hsv color is in range
					return true;
				}
			}
		}
		*/
		/*
		//[0, 58, 50] lower bound skin HSV
		//[30, 255, 255] upper bound skin HSV
		if ((myHsvColor.h >= 0) && (myHsvColor.h <= 30))
		{
			if ((myHsvColor.s >= 58) && (myHsvColor.s <= 255))
			{
				if ((myHsvColor.v >= 50) && (myHsvColor.v <= 255))
				{
					//if 3 conditions are meet, hsv color is in range
					return true;
				}
			}
		}
		*/
		/*
		//[0, 58, 50] lower bound skin HSV
		//[30, 174, 255] upper bound skin HSV
		if ((myHsvColor.h >= 0) && (myHsvColor.h <= 30))
		{
			if ((myHsvColor.s >= 58) && (myHsvColor.s <= 174))
			{
				if ((myHsvColor.v >= 50) && (myHsvColor.v <= 255))
				{
					//if 3 conditions are meet, hsv color is in range
					return true;
				}
			}
		}
		*/
		//spi
		if (((myHsvColor.h >= 0) && (myHsvColor.h <= 30)) || ((myHsvColor.h >= 108) && (myHsvColor.h <= 179)))
		{
			if ((myHsvColor.s >= 23) && (myHsvColor.s <= 255))
			{
				if ((myHsvColor.v >= 50) && (myHsvColor.v <= 255))
				{
					//if 3 conditions are meet, hsv color is in range
					return true;
				}
			}
		}

	}
	else if (hsvcolorrangetype == UTILITY_HSVCOLORRANGE_TYPE_RED)
	{
		//https://stackoverflow.com/questions/32522989/opencv-better-detection-of-red-color
		if (((myHsvColor.h >= 0) && (myHsvColor.h <= 10)) || ((myHsvColor.h >= 170) && (myHsvColor.h <= 180)))
		{
			if ((myHsvColor.s >= 70) && (myHsvColor.s <= 255))
			{
				if ((myHsvColor.v >= 50) && (myHsvColor.v <= 255))
				{
					//if 3 conditions are meet, hsv color is in range
					return true;
				}
			}
		}
	}
	else if (hsvcolorrangetype == UTILITY_HSVCOLORRANGE_TYPE_GREEN)
	{
		//https://stackoverflow.com/questions/31590499/opencv-android-green-color-detection
		//Green color is HSV space has H = 120 and it's in range [0, 360].
		//OpenCV halves the H values to fit the range [0,255], so H value instead of being in range [0, 360], is in range [0, 180]. S and V are still in range [0, 255].
		//As a consequence, the value of H for green is 60 = 120 / 2.
		// sensitivity is a int, typically set to 15 - 20 
		int sensitivity = 15;
		if ((myHsvColor.h >= (60 - sensitivity)) && (myHsvColor.h <= (60 + sensitivity)))
		{
			if ((myHsvColor.s >= 100) && (myHsvColor.s <= 255))
			{
				if ((myHsvColor.v >= 50) && (myHsvColor.v <= 255))
				{
					//if 3 conditions are meet, hsv color is in range
					return true;
				}
			}
		}
	}
	else if (hsvcolorrangetype == UTILITY_HSVCOLORRANGE_TYPE_BLUE)
	{
		//https://answers.opencv.org/question/90047/detecting-blue-color-in-this-image/
		if ((myHsvColor.h >= 100) && (myHsvColor.h <= 140))
		{
			if ((myHsvColor.s >= 150) && (myHsvColor.s <= 255))
			{
				if ((myHsvColor.v >= 0) && (myHsvColor.v <= 255))
				{
					//if 3 conditions are meet, hsv color is in range
					return true;
				}
			}
		}
	}
	else if (hsvcolorrangetype == UTILITY_HSVCOLORRANGE_TYPE_YELLOW)
	{
		//https://stackoverflow.com/questions/9179189/detect-yellow-color-in-opencv
		if ((myHsvColor.h >= 20) && (myHsvColor.h <= 30))
		{
			if ((myHsvColor.s >= 100) && (myHsvColor.s <= 255))
			{
				if ((myHsvColor.v >= 100) && (myHsvColor.v <= 255))
				{
					//if 3 conditions are meet, hsv color is in range
					return true;
				}
			}
		}
	}
	else if (hsvcolorrangetype == UTILITY_HSVCOLORRANGE_TYPE_ORANGE)
	{
		//https://realpython.com/python-opencv-color-spaces/
		if ((myHsvColor.h >= 1) && (myHsvColor.h <= 18))
		{
			if ((myHsvColor.s >= 190) && (myHsvColor.s <= 255))
			{
				if ((myHsvColor.v >= 200) && (myHsvColor.v <= 255))
				{
					//if 3 conditions are meet, hsv color is in range
					return true;
				}
			}
		}
	}
	return false;
}

//2020july04, spi, end


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
	i = ((len + 2) / 2)*sizeof(PVOID)+sizeof(PVOID);

	argv = (PCHAR*)GlobalAlloc(GMEM_FIXED,
		i + (len + 2)*sizeof(CHAR));

	_argv = (PCHAR)(((PUCHAR)argv) + i);

	argc = 0;
	argv[argc] = _argv;
	in_QM = FALSE;
	in_TEXT = FALSE;
	in_SPACE = TRUE;
	i = 0;
	j = 0;

	while (a = CmdLine[i]) {
		if (in_QM) {
			if (a == '\"') {
				in_QM = FALSE;
			}
			else {
				_argv[j] = a;
				j++;
			}
		}
		else {
			switch (a) {
			case '\"':
				in_QM = TRUE;
				in_TEXT = TRUE;
				if (in_SPACE) {
					argv[argc] = _argv + j;
					argc++;
				}
				in_SPACE = FALSE;
				break;
			case ' ':
			case '\t':
			case '\n':
			case '\r':
				if (in_TEXT) {
					_argv[j] = '\0';
					j++;
				}
				in_TEXT = FALSE;
				in_SPACE = TRUE;
				break;
			default:
				in_TEXT = TRUE;
				if (in_SPACE) {
					argv[argc] = _argv + j;
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


std::string space2underscore(std::string text)
{
	for (std::string::iterator it = text.begin(); it != text.end(); ++it)
	{
		if (*it == ' ')
		{
			*it = '_';
		}
	}
	return text;
}

std::string dot2dash(std::string text)
{
	for (std::string::iterator it = text.begin(); it != text.end(); ++it)
	{
		if (*it == '.')
		{
			*it = '-';
		}
	}
	return text;
}

std::string ampersand2underscore(std::string text)
{
	for (std::string::iterator it = text.begin(); it != text.end(); ++it)
	{
		if (*it == '&')
		{
			*it = '_';
		}
	}
	return text;
}

// copy in binary mode
bool copyFile(const char *SRC, const char* DEST)
{
	std::ifstream src(SRC, std::ios::binary);
	std::ofstream dest(DEST, std::ios::binary);
	dest << src.rdbuf();
	return src && dest;
}

bool hasEnding(std::string const &fullString, std::string const &ending) {
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	}
	else {
		return false;
	}
}

string getPathName(const string& s)
{
	char sep = '/';
#ifdef _WIN32
	sep = '\\';
#endif
	size_t i = s.rfind(sep, s.length());
	if (i != string::npos)
	{
		return(s.substr(0, i));
	}
	return("");
}

int GetHammingDistance(const std::string &a, const std::string &b, const bool bfromend)
{
	// Hamming distance is not defined for strings of different lengths.
	//ASSERT(a.length() == b.length());

	int distance = 0;

	if (bfromend == false)
	{
		//to iterate from start
		std::string::const_iterator a_it = a.begin();
		std::string::const_iterator b_it = b.begin();

		std::string::const_iterator a_end = a.end();
		std::string::const_iterator b_end = b.end();

		while (a_it != a_end && b_it != b_end)
		{
			if (*a_it != *b_it) return distance;
			++distance;
			++a_it; ++b_it;
		}
	}
	else
	{
		//to iterate from end
		std::string::const_iterator a_it = a.begin() + (a.size() - 1);
		std::string::const_iterator b_it = b.begin() + (b.size() - 1);

		std::string::const_iterator a_end = a.begin();
		std::string::const_iterator b_end = b.begin();

		while (a_it != a_end && b_it != b_end)
		{
			if (*a_it != *b_it) return distance;
			++distance;
			--a_it; --b_it;
		}
	}
	return distance;
}

string getcommonprefix(const std::string &a, const std::string &b)
{
	string commonprefix = "";
	int distance = GetHammingDistance(a, b);
	commonprefix = a.substr(0, distance);
	//patch
	//since folders and filenames both start with an "f", remove ending "f" from identical prefix
	string lasttwochar = commonprefix.substr(commonprefix.size() - 2);
	if (lasttwochar == "-f")
	{
		//remove last char (ending "f")
		commonprefix = commonprefix.substr(0, commonprefix.size() - 1);
	}
	return commonprefix;
}

string getcommonsuffix(const std::string &a, const std::string &b)
{
	string commonsuffix = "";
	int distance = GetHammingDistance(a, b, true); //true to iterate from end
	commonsuffix = a.substr(a.size() - distance);
	//patch
	//since folders and filenames both end with an "s", remove starting "s" from identical suffix
	string firsttwochar = commonsuffix.substr(0, 2);
	if (firsttwochar == "s_")
	{
		//remove first char (starting "s")
		commonsuffix = commonsuffix.substr(1);
	}
	return commonsuffix;
}

//erase first occurrence of given  substring from main string.
void erasesubstr(std::string & mainStr, const std::string & toErase)
{
	//search for the substring in string
	size_t pos = mainStr.find(toErase);
	if (pos != std::string::npos)
	{
		// If found then erase it from string
		mainStr.erase(pos, toErase.length());
	}
}

//2021oct15, spi, begin
string firstfolder(string path)
{
	string firstfolder = "";;
	size_t pos = 0, currentpos = 0;
	currentpos = path.find_first_of('\\', pos);
	if (currentpos != std::string::npos)
	{
		//first slash found
		firstfolder = path.substr(pos, currentpos - pos);
		if (isdriveletter(firstfolder))
		{
			//if drive letter, find next
			pos = currentpos + 1;
			currentpos = path.find_first_of('\\', pos);
			if (currentpos != std::string::npos)
			{
				firstfolder = path.substr(currentpos - pos);
			}
			else
			{
				firstfolder = path.substr(pos);
			}
		}
		else
		{
			//do nothing more
		}
	}
	else
	{
		//not even a drive letter nor ending slash, so it is a relative path and single folder for sure
		firstfolder = path;
	}
	return firstfolder;
}

bool isdriveletter(string path)
{
	bool result = false;
	size_t pos = 0, currentpos = 0;
	currentpos = path.find_first_of('\\', pos);
	if (currentpos != std::string::npos)
	{
		//first slash found
		if (path.size() != 3) return false;
	}
	else
	{
		//no slash
	}
	if (path[1] != ':') return false;
	int uppercasechar = toupper(path[0]);
	if ((uppercasechar >= 'A') && (uppercasechar <= 'Z')) result = true;
	return result;
}

//without the slash
string getdriveletter(string path)
{
	string driveletter = "";
	size_t pos = 0, currentpos = 0;
	currentpos = path.find_first_of('\\', pos);
	if (currentpos != std::string::npos)
	{
		//first slash found
		driveletter = path.substr(pos, currentpos - pos);

	}
	else
	{
		if (path.size() > 2)
		{
			driveletter = path.substr(0, 2);
		}
	}
	if (!isdriveletter(driveletter)) driveletter = "";
	return driveletter;
}

bool haswildcard(string path)
{
	bool result = false;
	size_t pos = 0, currentpos = 0;
	currentpos = path.find_first_of('*', pos);
	if (currentpos != std::string::npos)
	{
		//wildcard found
		result = true;
	}
	return result;
}

string removesubstr(string path, string toremove)
{
	// Search for the substring in string
	size_t pos = path.find(toremove);
	if (pos != std::string::npos)
	{
		// If found then erase it from string
		path.erase(pos, toremove.length());
	}
	return path;
}

bool isvalidfoldername(string path)
{
	bool result = false;
	if (path.empty()) return result;

	string directory = "";
	size_t pos = 0, currentpos = 0;
	while (currentpos != std::string::npos)
	{
		currentpos = path.find_first_of('\\', pos);
		if (currentpos == std::string::npos)
		{
			//slash not found
			directory = path.substr(pos);
		}
		else
		{
			//slash found
			directory = path.substr(pos, currentpos - pos);
		}
		if (pos == 0 && isdriveletter(directory))
		{
			//absolute path
			if (path.size() == 2) return true; //when only drive letter is specified
		}
		//todo
		//validate directory using regex or something
		pos = currentpos + 1;
	}
	//return result;
	return true;
}

bool isvalidfilename(string path)
{
	//check first condition, if it has a valid path pattern
	bool result = isvalidfoldername(getfolderfromfilename(path));
	if (result == true)
	{
		//check second condition, if it has an extension
		result = false;
		size_t pos = 0, currentpos = 0;
		//currentpos = path.find_first_of('.', pos);
		currentpos = path.find_last_of('.');
		if (currentpos != std::string::npos)
		{
			//extension found
			result = true;
		}
	}
	return result;
}

bool isvalidfilterstring(string path)
{
	return haswildcard(path);
}

//todo, add more extensions
std::vector<std::string> imageextensions{ "jpg", "jpeg", "bmp", "tif", "tiff", "png" };
bool isvalidimagefilename(string path)
{
	//check first condition, if path can be a valid filename
	bool result = isvalidfilename(path);
	if (result == true)
	{
		//check second condition, if it has an image extension
		result = false; //reset to false
		size_t pos = 0, currentpos = 0;
		//currentpos = path.find_first_of('.', pos);
		currentpos = path.find_last_of('.');
		if (currentpos != std::string::npos)
		{
			//dot found
			string dot_withextension = path.substr(currentpos);
			if (dot_withextension.size() > 2)
			{
				string extension = path.substr(currentpos + 1);
				std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
				for (int i = 0; i < imageextensions.size(); i++)
				{
					if (extension == imageextensions[i]) return true;
				}
			}
			//result = true;
		}
	}
	return result;
}

//todo, add more extensions
std::vector<std::string> txtextensions{ "txt", "bak", "nfo", "cfg" };
bool isvalidtxtfilename(string path)
{
	//check first condition, if path can be a valid filename
	bool result = isvalidfilename(path);
	if (result == true)
	{
		//check second condition, if it has an txt extension
		result = false; //reset to false
		size_t pos = 0, currentpos = 0;
		//currentpos = path.find_first_of('.', pos);
		currentpos = path.find_last_of('.');
		if (currentpos != std::string::npos)
		{
			//dot found
			string dot_withextension = path.substr(currentpos);
			if (dot_withextension.size() > 2)
			{
				string extension = path.substr(currentpos + 1);
				std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
				for (int i = 0; i < txtextensions.size(); i++)
				{
					if (extension == txtextensions[i]) return true;
				}
			}
			//result = true;
		}
	}
	return result;
}

bool isvalidtxtfilename_containingfoldernames(string path)
{
	//check first condition, if path is a valid txt filename
	bool result = isvalidtxtfilename(path);
	if (result)
	{
		ifstream ifs(path);
		string temp;
		result = false; //reset
		while (getline(ifs, temp))
		{
			if (!temp.empty())
			{
				if ((isvalidfilterstring(temp) == false) &&
					(isvalidfilename(temp) == false) &&
					(isvalidfoldername(temp) == true))
				{
					//ok, keep checking
				}
				else
				{
					return false;
				}
			}
		}
		result = true;
	}
	return result;
}

bool isvalidtxtfilename_containingfilenames(string path)
{
	//check first condition, if path is a valid txt filename
	bool result = isvalidtxtfilename(path);
	if (result)
	{
		ifstream ifs(path);
		string temp;
		result = false; //reset
		while (getline(ifs, temp))
		{
			if (!temp.empty())
			{
				if ((isvalidfilterstring(temp) == false) &&
					(isvalidfilename(temp) == true))
				{
					//ok, keep checking
				}
				else
				{
					return false;
				}
			}
		}
		result = true;
	}
	return result;
}

bool isvalidtxtfilename_containingfilterstrings(string path)
{
	//check first condition, if path is a valid txt filename
	bool result = isvalidtxtfilename(path);
	if (result)
	{
		ifstream ifs(path);
		string temp;
		result = false; //reset
		while (getline(ifs, temp))
		{
			if (!temp.empty())
			{
				if (isvalidfilterstring(temp) == true)
				{
					//ok, keep checking
				}
				else
				{
					return false;
				}
			}
		}
		result = true;
	}
	return result;
}

string getfolderfromfilename(string filename)
{
	string folder = "";
	size_t found;
	found = filename.find_last_of("/\\");
	if (found != string::npos)
	{
		//absolute path
		folder = filename.substr(0, found);
		//filename.substr(found + 1);
	}
	else
	{
		//relative path or filename
		folder = filename;
	}
	return folder;
}

void removeduplicates(std::vector<string>& vec)
{
	std::sort(vec.begin(), vec.end());
	vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
}
//2021oct15, spi, end



//2021nov16, spi, begin
//for windows and linux, see http link
//https://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process

DWORDLONG getmemory_totalavailable_physical()
{
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
	return totalPhysMem;
}

DWORDLONG getmemory_totalavailable_virtual()
{
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	DWORDLONG totalVirtualMem = memInfo.ullTotalPageFile;
	//Note: The name "TotalPageFile" is a bit misleading here. 
	//In reality this parameter gives the "Virtual Memory Size", 
	//which is size of swap file plus installed RAM.
	return totalVirtualMem;
}

DWORDLONG getmemory_totalused_physical()
{
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	DWORDLONG physMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
	return physMemUsed;
}

DWORDLONG getmemory_totalused_virtual()
{
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	DWORDLONG virtualMemUsed = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;
	return virtualMemUsed;
}

SIZE_T getmemory_totalusedbyme_physical()
{
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	//SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;
	//return virtualMemUsedByMe;
	SIZE_T physMemUsedByMe = pmc.WorkingSetSize;
	return physMemUsedByMe;
}

SIZE_T getmemory_totalusedbyme_virtual()
{
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	//SIZE_T physMemUsedByMe = pmc.WorkingSetSize;
	//return physMemUsedByMe;
	SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;
	return virtualMemUsedByMe;
}


spilogfile::spilogfile()
{
	init();
	ofs.open(SPILOGFILE_FILENAME, std::ofstream::out);
}

spilogfile::spilogfile(string filename, bool timestamped/* = false*/)
{
	if (filename.empty() && timestamped == false)
	{
		::spilogfile();
	}
	else
	{
		init();
		if (filename.empty()) filename = SPILOGFILE_FILENAME;
		string finalfilename;
		if (timestamped)
		{
			finalfilename = getcurrentdatetime(GETCURRENTDATETIME_YYYYMMDD_HHMMSS_SUBSEC_NODASHNODOTNOSEMICOLON);
			finalfilename += "_";
			finalfilename += filename;
		}
		ofs.open(finalfilename, std::ofstream::out);
	}
	return;
}

spilogfile::~spilogfile()
{
	if (ofs.is_open()) ofs.close();
}

bool spilogfile::init()
{
	//read from registry
	prev_nowstamp_ms = RegReadDWORD(SPITONICMIDIINSTRUMENTSYNTHSPECTRUMWIN32_VS2019_HKEYROOT,
		SPITONICMIDIINSTRUMENTSYNTHSPECTRUMWIN32_VS2019_REGSUBKEY, L"spilogfile_prev_nowstamp_ms");
	counter_subsec = RegReadDWORD(SPITONICMIDIINSTRUMENTSYNTHSPECTRUMWIN32_VS2019_HKEYROOT,
		SPITONICMIDIINSTRUMENTSYNTHSPECTRUMWIN32_VS2019_REGSUBKEY, L"spilogfile_counter_subsec");
	/*
	counter_submillisec = 0;
	*/
	return true;
}

bool spilogfile::write(string mystring, bool to_cout/*=false*/)
{
	if (!mystring.empty())
	{
		if (ofs.is_open())
		{
			ofs << mystring;
			//2021nov29, spi, begin
			ofs.flush();
			//2021nov29, spi, end
		}
		if (to_cout)
		{
			cout << mystring;
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool spilogfile::writeline(string mystring, bool to_cout/*=false*/)
{
	mystring += "\n";
	return write(mystring, to_cout);
}

const std::string spilogfile::getcurrentdatetime(int stringformat /*=GETCURRENTDATETIME_YYYYMMDD_HHMMSS_SUBSEC_NODASHNODOTNOSEMICOLON*/)
{
	string mystring = "";
	DWORD nowstamp_ms = GetTickCount();
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
	// for more information about date/time format
	if (stringformat == GETCURRENTDATETIME_YYYYMMDD_HHMMSS_DASHDOTSEMICOLON)
	{
		//format is YYYY-MM-DD.HH:mm:ss
		strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
		mystring = buf;
	}
	else if (stringformat == GETCURRENTDATETIME_YYYYMMDD_HHMMSS_NODASHNODOTNOSEMICOLON)
	{
		//for a format like 2021Feb16_164917
		strftime(buf, sizeof(buf), "%Y%b%d_%H%M%S", &tstruct);
		mystring = buf;
	}
	else if (stringformat == GETCURRENTDATETIME_YYYYMMDD_HHMMSS_SUBSEC_NODASHNODOTNOSEMICOLON)
	{
		if (abs((long long)(nowstamp_ms - prev_nowstamp_ms)) < 1000)
		{
			counter_subsec++;
		}
		else
		{
			counter_subsec = 0;
		}
		//write to registry
		RegWriteDWORD(SPITONICMIDIINSTRUMENTSYNTHSPECTRUMWIN32_VS2019_HKEYROOT,
			SPITONICMIDIINSTRUMENTSYNTHSPECTRUMWIN32_VS2019_REGSUBKEY, L"spilogfile_counter_subsec", counter_subsec);
		WCHAR charbuf[80];
		wsprintf(charbuf, L"_%03d", counter_subsec);
		/*
		if ((nowstamp_ms - prev_nowstamp_ms)<1)
		{
		counter_submillisec++;
		}
		else
		{
		counter_submillisec = 0;
		}
		*/
		//for a format like 2021Feb16_164917_000_pointset_debug.txt
		strftime(buf, sizeof(buf), "%Y%b%d_%H%M%S", &tstruct);
		mystring = buf;
		mystring += utf8_encode(charbuf);
	}
	prev_nowstamp_ms = nowstamp_ms;
	//write to registry
	RegWriteDWORD(SPITONICMIDIINSTRUMENTSYNTHSPECTRUMWIN32_VS2019_HKEYROOT,
		SPITONICMIDIINSTRUMENTSYNTHSPECTRUMWIN32_VS2019_REGSUBKEY, L"spilogfile_prev_nowstamp_ms", prev_nowstamp_ms);
	return mystring;
}


//uses dir command to populate all found filenames into supplied txt file and supplied string vector
//searches for files matching supplied filter in supplied directory, including all subfolders by default
//returns files count
int getfilenamesfromfolder(std::string folderpath, std::string filefilter, std::string outputtxtfilename, vector<std::string> &outputstringvector, bool includesubfolders/*=true*/)
{
	//0) execute cmd line to get all folder's iamge filenames
	string quote = "\"";
	/* //for now, no validation on filefilter
	if (filefilter.substr(1, 1) != ".") filefilter += ".";
	if (filefilter.substr(0, 1) != "*") filefilter += "*";
	*/
	string pathfilter;
	bool folderpathslashterminated = false;
	if (folderpath.substr(folderpath.size() - 1, 1) == "\\")
	{
		folderpathslashterminated = true;
		pathfilter = folderpath + filefilter;
	}
	else
	{
		pathfilter = folderpath + "\\" + filefilter;
	}

	string systemcommand;
	//systemcommand = "DIR " + quote + pathfilter + quote + "/B /O:N > wsic_filenames.txt"; //wsip tag standing for wav set (library) instrumentset (class) populate (function)
	//systemcommand = "DIR " + quote + pathfilter + quote + "/B /S /O:N > spislideshow_filenames.txt"; // /S for adding path into "wsic_filenames.txt"
	//systemcommand = "DIR " + quote + pathfilter + quote + "/B /S /O:N > " + outputtxtfilename; // /S for adding path into "wsic_filenames.txt"
	string subfolderflagstring = "";
	if (includesubfolders == true) subfolderflagstring = "/S ";
	systemcommand = "DIR " + quote + pathfilter + quote + "/B " + subfolderflagstring + "/O:N > " + outputtxtfilename; // /S for adding path into "wsic_filenames.txt"
	system(systemcommand.c_str());

	//2) load in all "spiss_filenames.txt" file
	//vector<string> global_txtfilenames;
	//ifstream ifs("spislideshow_filenames.txt");
	ifstream ifs(outputtxtfilename);
	string temp;
	while (getline(ifs, temp))
	{
		if (!temp.empty())
		{
			if (includesubfolders == true)
			{
				//global_txtfilenames.push_back(temp);
				outputstringvector.push_back(temp);
			}
			else
			{
				//txtfilenames.push_back(path + "\\" + temp);
				if (folderpathslashterminated == true)
				{
					outputstringvector.push_back(folderpath + temp);
				}
				else
				{
					outputstringvector.push_back(folderpath + "\\" + temp);
				}
			}
		}
	}
	return outputstringvector.size();
}
//2021nov16, spi, end




//2021nov29, spi, begin
//for submonitors and subvorogui
spisubrect::spisubrect(RECT referencerect, int numberofsubrect, bool horizontalfirst/* = true*/, bool wraparound/* = true*/, bool noorphans/* = false*/, int gridtypeflag/* = SPISUBRECT_GRIDLIKE*/)
{
	p_spilogfile = NULL;
	if(0)//if (SPIWINDOWTRANSPARENTMIDIVOROGUISPECTRUMTIMEFRAME_VS2017_SPISUBRECT_DEBUG)
	{
		p_spilogfile = DBG_NEW spilogfile("spisubrect_debug.txt", true);
	}

	if (numberofsubrect > 0)
	{
		_horizontalfirst = horizontalfirst;
		_wraparound = wraparound;
		if (gridtypeflag & SPISUBRECT_GRIDLIKE)
		{
			_columns = ceil(sqrt((float)numberofsubrect));
		}
		else if(gridtypeflag & SPISUBRECT_LISTLIKE_VERTICAL)
		{
			_columns = 1;
		}
		else if (gridtypeflag & SPISUBRECT_LISTLIKE_HORIZONTAL)
		{
			_columns = numberofsubrect;
		}
		else
		{
			//defaults to SPISUBRECT_GRIDLIKE, for safety
			_columns = ceil(sqrt((float)numberofsubrect));
		}
		_fullrows = numberofsubrect / _columns;
		_orphans = numberofsubrect % _columns;   // how many 'odd-sized' ones on our bottom row.
		if (noorphans == true && _orphans != 0)
		{
			_fullrows += 1;
			_orphans = 0;
		}
		if (_orphans != 0)
		{
			_rows = _fullrows + 1;
		}
		else
		{
			_rows = _fullrows;
		}
		//calculate output width and height
		_width = (referencerect.right - referencerect.left) / _columns; //original.width/ columns;
		_height = (referencerect.bottom - referencerect.top) / (_orphans == 0 ? _fullrows : (_fullrows + 1)); //original.height / (orphans == 0 ? fullrows : (fullrows+1)); // reduce height if there are orphans
		//calculate output rectangles
		RECT output;
		if (horizontalfirst)
		{
			//2.1 2.2
			//2.3 2.4
			int i = -1;
			for (int y = 0; y < _rows; ++y)  //for (int y = 0; y < _fullrows; ++y) //for (int x = 0; x < _columns; ++x) 
			{
				for (int x = 0; x < _columns; ++x) //for (int y = 0; y < _fullrows; ++y)
				{
					i++;
					if ((i < numberofsubrect) || (noorphans == true))
					{
						output.left = referencerect.left + x * _width;
						output.top = referencerect.top + y * _height;
						output.right = output.left + _width;
						output.bottom = output.top + _height;
						_rectvector.push_back(output);
					}
				}
			}
		}
		else
		{
			//2.1 2.3
			//2.2 2.4
			int i = -1;
			for (int x = 0; x < _columns; ++x) //for (int y = 0; y < fullrows; ++y)
			{
				for (int y = 0; y < _rows; ++y)//for (int y = 0; y < _fullrows; ++y) //for (int x = 0; x < columns; ++x)
				{
					i++;
					if ((i < numberofsubrect) || (noorphans == true))
					{
						output.left = referencerect.left + x * _width;
						output.top = referencerect.top + y * _height;
						output.right = output.left + _width;
						output.bottom = output.top + _height;
						_rectvector.push_back(output);
					}
				}
			}
		}
		if (p_spilogfile)
		{
			for (vector<RECT>::iterator it = _rectvector.begin(); it < _rectvector.end(); it++)
			{
				RECT myRECT = *it;
				stringstream ss;
				ss << "spisubrect initialized with" << endl;
				ss << "(left, top, width, height) = (" << myRECT.left << ", " << myRECT.top << ", " << abs(myRECT.right - myRECT.left) << ", " << abs(myRECT.bottom - myRECT.top) << ")" << endl;
				p_spilogfile->write(ss.str());
			}
			p_spilogfile->writeline("");
		}
	}
	else
	{
		_horizontalfirst = horizontalfirst;
		_wraparound = wraparound;
		_columns = 0;
		_rows = 0;
		_fullrows = 0;
		_orphans = 0;
		_width = 0;
		_height = 0;
	}
}

spisubrect::~spisubrect()
{
	if (p_spilogfile) delete p_spilogfile;
}

int spisubrect::size()
{
	return _rectvector.size();
}

int spisubrect::cols()
{
	return _columns;
}

int spisubrect::rows()
{
	return (_orphans == 0 ? _fullrows : (_fullrows + 1));
}

SIZE spisubrect::sizerect()
{
	SIZE size;
	size.cx = _width;
	size.cy = _height;
	return size;
}

RECT spisubrect::getrect(int index) //zero-based index
{
	RECT rect = {};
	if (index > -1 && index < _rectvector.size())
	{
		rect = _rectvector[index];
	}
	if (p_spilogfile)
	{
		RECT myRECT = rect;
		stringstream ss;
		ss << "spisubrect getrect(" << index << ") returns" << endl;
		ss << "(left, top, width, height) = (" << myRECT.left << ", " << myRECT.top << ", " << abs(myRECT.right - myRECT.left) << ", " << abs(myRECT.bottom - myRECT.top) << ")" << endl;
		p_spilogfile->write(ss.str());
		p_spilogfile->writeline("");
	}
	return rect;
}

POINT spisubrect::getrectcenter(int index) //zero-based index
{
	POINT point = {0,0};
	RECT rect = getrect(index);
	point.x = (rect.left + rect.right) / 2;
	point.y = (rect.top + rect.bottom) / 2;
	return point;
}

int spisubrect::getindex(int col, int row) //1-based col and row ids
{
	int index = -1; //no match
	if ((col > 0 && col <= cols()) && (row > 0 && row <= rows()))
	{
		int index = 0;
		if (_horizontalfirst)
		{
			index = (row - 1)*cols() + (col - 1);
		}
		else
		{
			index = (col - 1)*rows() + (row - 1);
		}
	}
	return index;
}

RECT spisubrect::getrect(int col, int row) //1-based col and row ids
{
	RECT rect = {};
	if ((col > 0 && col <= cols()) && (row > 0 && row <= rows()))
	{
		int index = getindex(col, row);
		rect = getrect(index);
	}
	return rect;
}

POINT spisubrect::getrectcenter(int col, int row) //1-based col and row ids
{
	POINT point = { 0,0 };
	RECT rect = getrect(col, row);
	point.x = (rect.left + rect.right) / 2;
	point.y = (rect.top + rect.bottom) / 2;
	return point;
}

int spisubrect::getindex(RECT fromrect)
{
	int index = -1; //no match
	RECT rect;
	int i = -1;
	for (std::vector<RECT>::iterator it = _rectvector.begin(); it != _rectvector.end(); ++it)
	{
		i++;
		rect = *it;
		//is fromrect equal to rect?
		bool bequal = EqualRect(&fromrect, &rect);
		if (bequal)
		{
			index = i;
		}
		else
		{
			//is fromrect contained inside rect?
			POINT topleft;
			topleft.x = fromrect.left;
			topleft.y = fromrect.top;
			POINT bottomright;
			bottomright.x = fromrect.right;
			bottomright.y = fromrect.bottom;
			bool btopleftcontained = PtInRect(&rect, topleft);
			if (btopleftcontained && PtInRect(&rect, bottomright))
			{
				index = i;
			}
		}
	}
	if (p_spilogfile)
	{
		RECT myRECT = fromrect;
		stringstream ss;
		ss << "spisubrect getindex(fromrect) where fromrect is" << endl;
		ss << "(left, top, width, height) = (" << myRECT.left << ", " << myRECT.top << ", " << abs(myRECT.right - myRECT.left) << ", " << abs(myRECT.bottom - myRECT.top) << ")" << endl;
		ss << "returns index " << index << endl;
		p_spilogfile->write(ss.str());
		p_spilogfile->writeline("");
	}
	return index;
}

bool spisubrect::getcolrow(int index, int &col, int &row)
{
	bool result = false;
	if (index > -1 && index < _rectvector.size())
	{
		int i = -1;
		if (_horizontalfirst)
		{
			//2.1 2.2
			//2.3 2.4
			for (int y = 0; y < _rows; ++y) //for (int y = 0; y < _fullrows; ++y) //for (int x = 0; x < _columns; ++x) 
			{
				for (int x = 0; x < _columns; ++x) //for (int y = 0; y < _fullrows; ++y)
				{
					i++;
					if (i == index)
					{
						col = x + 1; //1-based id
						row = y + 1; //1-based id
						return true;
					}
				}
			}
		}
		else
		{
			//2.1 2.3
			//2.2 2.4
			for (int x = 0; x < _columns; ++x) //for (int y = 0; y < fullrows; ++y)
			{
				for (int y = 0; y < _rows; ++y) //for (int y = 0; y < _fullrows; ++y) //for (int x = 0; x < columns; ++x)
				{
					i++;
					if (i == index)
					{
						col = x + 1; //1-based id
						row = y + 1; //1-based id
						return true;
					}
				}
			}
		}

	}
	return result;
}

RECT spisubrect::getnextrect(RECT fromrect)
{
	RECT rect = {};
	int index = getindex(fromrect);
	if (index == -1)
	{
		//if fromrect not found within _rectvector, match first
		rect = getrect(0); //get first rect
	}
	else if (index > -1 && index < _rectvector.size())
	{
		if (index < (_rectvector.size() - 1))
		{
			rect = getrect(index + 1); //get next rect
		}
		else
		{
			//when fromrect matches the last rect
			if (_wraparound == true)
			{
				//return first rect
				rect = getrect(0);
			}
			else
			{
				//do not wrap around, leave returned rect empty
			}
		}
	}
	if (p_spilogfile)
	{
		RECT myRECT = fromrect;
		stringstream ss;
		ss << "spisubrect getnextrect(fromrect) where fromrect is" << endl;
		ss << "(left, top, width, height) = (" << myRECT.left << ", " << myRECT.top << ", " << abs(myRECT.right - myRECT.left) << ", " << abs(myRECT.bottom - myRECT.top) << ")" << endl;
		ss << "returns rect " << endl;
		myRECT = rect;
		ss << "(left, top, width, height) = (" << myRECT.left << ", " << myRECT.top << ", " << abs(myRECT.right - myRECT.left) << ", " << abs(myRECT.bottom - myRECT.top) << ")" << endl;
		p_spilogfile->write(ss.str());
		p_spilogfile->writeline("");
	}

	return rect;
}

POINT spisubrect::getnextrectcenter(RECT fromrect) //1-based col and row ids
{
	POINT point = { 0,0 };
	RECT rect = getnextrect(fromrect);
	point.x = (rect.left + rect.right) / 2;
	point.y = (rect.top + rect.bottom) / 2;
	return point;
}
//2021nov29, spi, end





//2021nov29, spi, begin
RECT MonitorRects::FindMonitorRect(HMONITOR hmon)
{
	RECT rect = {};
	int i = -1;
	for (i = 0; i < hMonitors.size(); ++i)
	{
		if (hmon == hMonitors[i]) break;
	}
	if (i != -1)
	{
		//use this monitor
		std::map<HMONITOR, RECT>::iterator it;
		it = hmapMonitors.find(hmon);
		if (it != hmapMonitors.end())
		{
			rect = (*it).second;
		}
	}
	if (p_spilogfile)
	{
		RECT myRECT = rect;
		stringstream ss;
		ss << "FindMonitorRect(hmon) with hmon " << hmon << " returns" << endl;
		ss << "(left, top, width, height) = (" << myRECT.left << ", " << myRECT.top << ", " << abs(myRECT.right - myRECT.left) << ", " << abs(myRECT.bottom - myRECT.top) << ")" << endl;
		p_spilogfile->write(ss.str());
		p_spilogfile->writeline("");
	}
	return rect; //invalid hmon, return empty rect 
}

RECT MonitorRects::FindMonitorRect(HWND hwnd)
{
	HMONITOR hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
	return FindMonitorRect(hmon);
}

RECT MonitorRects::FindSubMonitorRect(HWND hwnd, int* p_index)
{
	if (p_index != NULL) *p_index = -1;
	RECT rectempty = {};
	RECT rectwnd = {};
	RECT rect = {};
	//monitor rect
	HMONITOR hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
	rect = FindMonitorRect(hmon);
	//submonitor rect
	bool bshift = GetKeyState(VK_SHIFT) & 0x8000;
	bool bcontrol = GetKeyState(VK_CONTROL) & 0x8000;
	int numberofsubrect = 1;
	if (bshift == true) numberofsubrect = MONITORRECTS_NUMBEROFSUBMONITORS_SHIFTDOW0N;
	if (bcontrol == true) numberofsubrect = MONITORRECTS_NUMBEROFSUBMONITORS_CONTROLDOWN;
	if (numberofsubrect == 1) return rect;
	spisubrect myspisubrect(rect, numberofsubrect, SPISUBRECT_HORIZONTALFIRST, SPISUBRECT_NOWRAPAROUND, SPISUBRECT_NOORPHANS);
	bool result = GetWindowRect(hwnd, &rectwnd);
	if (result == false) return rect;
	int index = myspisubrect.getindex(rectwnd);
	if (p_index != NULL) *p_index = index;
	if (index == -1)
	{
		//is full monitor rect or arbitrary rect
		if (p_spilogfile)
		{
			RECT myRECT = rect;
			stringstream ss;
			ss << "FindSubMonitorRect(hwnd) with hwnd " << hwnd << " returns" << endl;
			ss << "(left, top, width, height) = (" << myRECT.left << ", " << myRECT.top << ", " << abs(myRECT.right - myRECT.left) << ", " << abs(myRECT.bottom - myRECT.top) << ")" << endl;
			p_spilogfile->write(ss.str());
			p_spilogfile->writeline("");
		}
		return rect;
	}
	//actual submonitor rect
	return myspisubrect.getrect(index);
}

RECT MonitorRects::FindNextMonitorRect(HMONITOR hmon, HMONITOR* p_nexthmon)
{
	if (p_nexthmon != NULL) *p_nexthmon = NULL;
	RECT rect = {};
	int i = -1;
	for (i = 0; i < hMonitors.size(); ++i)
	{
		if (hmon == hMonitors[i]) break;
	}
	i++; //take the next monitor
	if (i > (hMonitors.size() - 1)) i = 0;
	HMONITOR nexthmon = hMonitors[i];
	if (p_nexthmon != NULL) *p_nexthmon = nexthmon;
	std::map<HMONITOR, RECT>::iterator it;
	it = hmapMonitors.find(nexthmon);
	if (it != hmapMonitors.end())
	{
		rect = (*it).second;
	}
	if (p_spilogfile)
	{
		RECT myRECT = rect;
		stringstream ss;
		ss << "FindNextMonitorRect(hmon) with hmon " << hmon << " returns" << endl;
		ss << "(left, top, width, height) = (" << myRECT.left << ", " << myRECT.top << ", " << abs(myRECT.right - myRECT.left) << ", " << abs(myRECT.bottom - myRECT.top) << ")" << endl;
		p_spilogfile->write(ss.str());
		p_spilogfile->writeline("");
	}
	return rect;
}

RECT MonitorRects::FindPreviousMonitorRect(HMONITOR hmon, HMONITOR* p_previoushmon)
{
	if (p_previoushmon != NULL) *p_previoushmon = NULL;
	RECT rect = {};
	int i = -1;
	for (i = 0; i < hMonitors.size(); ++i)
	{
		if (hmon == hMonitors[i]) break;
	}
	i--; //take the previous monitor
	if (i < 0) i = hMonitors.size() - 1;
	HMONITOR prevhmon = hMonitors[i];
	if (p_previoushmon != NULL) *p_previoushmon = prevhmon;
	std::map<HMONITOR, RECT>::iterator it;
	it = hmapMonitors.find(prevhmon);
	if (it != hmapMonitors.end())
	{
		rect = (*it).second;
	}
	if (p_spilogfile)
	{
		RECT myRECT = rect;
		stringstream ss;
		ss << "FindPreviousMonitorRect(hmon) with hmon " << hmon << " returns" << endl;
		ss << "(left, top, width, height) = (" << myRECT.left << ", " << myRECT.top << ", " << abs(myRECT.right - myRECT.left) << ", " << abs(myRECT.bottom - myRECT.top) << ")" << endl;
		p_spilogfile->write(ss.str());
		p_spilogfile->writeline("");
	}
	return rect;
}

RECT MonitorRects::FindNextSubMonitorRect(HWND hwnd)
{
	RECT rectempty = {};
	RECT rectwnd = {};
	RECT monrect = {};
	RECT nextmonrect = {};
	RECT submonrect = {};
	RECT nextsubmonrect = {};
	//actual monitor
	HMONITOR hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
	monrect = FindMonitorRect(hmon);
	//next monitor
	HMONITOR nexthmon = NULL;
	nextmonrect = FindNextMonitorRect(hmon, &nexthmon);

	//actual submonitor
	bool bshift = GetKeyState(VK_SHIFT) & 0x8000;
	bool bcontrol = GetKeyState(VK_CONTROL) & 0x8000;
	int numberofsubrect = 1;
	if (bshift == true) numberofsubrect = MONITORRECTS_NUMBEROFSUBMONITORS_SHIFTDOW0N;
	if (bcontrol == true) numberofsubrect = MONITORRECTS_NUMBEROFSUBMONITORS_CONTROLDOWN;
	if (numberofsubrect == 1)
	{
		nextsubmonrect = nextmonrect;
	}
	else
	{
		spisubrect myspisubrect(monrect, numberofsubrect, SPISUBRECT_HORIZONTALFIRST, SPISUBRECT_NOWRAPAROUND, SPISUBRECT_NOORPHANS);
		int submonindex;
		submonrect = FindSubMonitorRect(hwnd, &submonindex);
		if ( (bshift == true) && (bcontrol == true) )
		{
			//random location
			int randomindex = -1;
			while ( (randomindex == -1) || (randomindex == submonindex) )
			{
				randomindex = RandomInt(0, numberofsubrect - 1);
				nextsubmonrect = myspisubrect.getrect(randomindex);
			}
		}
		else
		{
			//next submonitor
			if (submonindex == -1)
			{
				//get this monitor's first submonitor
				nextsubmonrect = myspisubrect.getrect(0);
			}
			else
			{
				submonindex++; //even if submonindex is -1, incrementing will give 0 so it is safe and what we want
				if (submonindex > (myspisubrect.size() - 1))
				{
					//get next monitor's first submonitor
					spisubrect mynextmonspisubrect(nextmonrect, numberofsubrect, SPISUBRECT_HORIZONTALFIRST, SPISUBRECT_NOWRAPAROUND, SPISUBRECT_NOORPHANS);
					nextsubmonrect = mynextmonspisubrect.getrect(0);
				}
				else
				{
					//get this monitor's next submonitor
					nextsubmonrect = myspisubrect.getrect(submonindex);
				}
			}
		}
	}
	if (p_spilogfile)
	{
		RECT myRECT = nextsubmonrect;
		stringstream ss;
		ss << "FindNextSubMonitorRect(hwnd) with hwnd " << hwnd << " returns" << endl;
		ss << "(left, top, width, height) = (" << myRECT.left << ", " << myRECT.top << ", " << abs(myRECT.right - myRECT.left) << ", " << abs(myRECT.bottom - myRECT.top) << ")" << endl;
		p_spilogfile->write(ss.str());
		p_spilogfile->writeline("");
	}
	return nextsubmonrect;
}

RECT MonitorRects::FindPreviousSubMonitorRect(HWND hwnd)
{
	RECT rectempty = {};
	RECT rectwnd = {};
	RECT monrect = {};
	RECT prevmonrect = {};
	RECT submonrect = {};
	RECT prevsubmonrect = {};
	//actual monitor
	HMONITOR hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
	monrect = FindMonitorRect(hmon);
	//prev monitor
	HMONITOR prevhmon = NULL;
	prevmonrect = FindPreviousMonitorRect(hmon, &prevhmon);

	//actual submonitor
	bool bshift = GetKeyState(VK_SHIFT) & 0x8000;
	bool bcontrol = GetKeyState(VK_CONTROL) & 0x8000;
	int numberofsubrect = 1;
	if (bshift == true) numberofsubrect = MONITORRECTS_NUMBEROFSUBMONITORS_SHIFTDOW0N;
	if (bcontrol == true) numberofsubrect = MONITORRECTS_NUMBEROFSUBMONITORS_CONTROLDOWN;
	if (numberofsubrect == 1)
	{
		prevsubmonrect = prevmonrect;
	}
	else
	{
		spisubrect myspisubrect(monrect, numberofsubrect, SPISUBRECT_HORIZONTALFIRST, SPISUBRECT_NOWRAPAROUND, SPISUBRECT_NOORPHANS);
		int submonindex;
		submonrect = FindSubMonitorRect(hwnd, &submonindex);
		if ( (bshift == true) && (bcontrol == true) )
		{
			//random location
			int randomindex = -1;
			while ((randomindex == -1) || (randomindex == submonindex))
			{
				randomindex = RandomInt(0, numberofsubrect - 1);
				prevsubmonrect = myspisubrect.getrect(randomindex);
			}
		}
		else
		{
			//prev submonitor
			if (submonindex == -1)
			{
				//get this monitor's last submonitor
				prevsubmonrect = myspisubrect.getrect(myspisubrect.size() - 1);
			}
			else
			{
				submonindex--; //if submonindex is -1, decrementing will give -2, if submonindex is 0, decrementing will give -1
				if (submonindex < 0)
				{
					//get previous monitor's last submonitor
					spisubrect myprevmonspisubrect(prevmonrect, numberofsubrect, SPISUBRECT_HORIZONTALFIRST, SPISUBRECT_NOWRAPAROUND, SPISUBRECT_NOORPHANS);
					prevsubmonrect = myprevmonspisubrect.getrect(myprevmonspisubrect.size() - 1);
				}
				else
				{
					//get this monitor's previous submonitor
					prevsubmonrect = myspisubrect.getrect(submonindex);
				}
			}
		}
	}
	if (p_spilogfile)
	{
		RECT myRECT = prevsubmonrect;
		stringstream ss;
		ss << "FindPreviousSubMonitorRect(hwnd) with hwnd " << hwnd << " returns" << endl;
		ss << "(left, top, width, height) = (" << myRECT.left << ", " << myRECT.top << ", " << abs(myRECT.right - myRECT.left) << ", " << abs(myRECT.bottom - myRECT.top) << ")" << endl;
		p_spilogfile->write(ss.str());
		p_spilogfile->writeline("");
	}
	return prevsubmonrect;

}

int MonitorRects::CountMonitorRectSize(RECT rect)
{
	int count = 0;
	std::map<HMONITOR, RECT>::iterator it;
	for (it = hmapMonitors.begin(); it != hmapMonitors.end(); it++)
	{
		RECT monitorrect = (*it).second;
		//compare sizes
		SIZE size = { abs(rect.right - rect.left), abs(rect.bottom - rect.top) };
		SIZE monitorsize = { abs(monitorrect.right - monitorrect.left) , abs(monitorrect.bottom - monitorrect.top) };
		//relative rects
		RECT relativerect = { 0, 0, size.cx, size.cy };
		RECT relativemonitorrect = { 0, 0, monitorsize.cx, monitorsize.cy };
		bool equal = EqualRect(&relativerect, &relativemonitorrect);
		if (equal) count++;
	}
	return count;
}

int MonitorRects::CountCanvasSize(SIZE size)
{
	int count = 0;
	RECT rect = {0, 0, size.cx, size.cy};
	std::vector<SIZE>::iterator it;
	for (it = canvassizesvector.begin(); it != canvassizesvector.end(); it++)
	{
		SIZE canvassize = (*it);
		RECT canvasrect = {0, 0, canvassize.cx, canvassize.cy};
		bool equal = EqualRect(&rect, &canvasrect);
		if (equal) count++;
	}
	return count;
}

int MonitorRects::MatchCanvasSize(SIZE size)
{
	int index = -1; //no match found
	if (size.cx > 0 && size.cy > 0)
	{
		RECT rect = { 0, 0, size.cx, size.cy };
		std::vector<SIZE>::iterator it;
		int ind = -1;
		for (it = canvassizesvector.begin(); it != canvassizesvector.end(); it++)
		{
			ind++;
			SIZE canvassize = (*it);
			RECT canvasrect = { 0, 0, canvassize.cx, canvassize.cy };
			bool equal = EqualRect(&rect, &canvasrect);
			if (equal) index = ind;
		}
	}
	return index;
}

int MonitorRects::MatchCanvasSize(LONG width, LONG height)
{
	SIZE size = {width, height};
	return MatchCanvasSize(size);
}

//2021nov29, spi, end



//2021dec29, spi, begin
bool bytebuffer_cell_setpixel(int icell, int jcell, BYTE* buffer, int width, int height, int numbyteperpixel, RGBQUAD pixelcolor/*={0,0,0,0}*/, int cell_nx/*=1*/, int cell_ny/*=1*/,
	int pixel_inverseprobability/*=1*/, int cell_inverseprobability/*=1*/,
	RGBQUAD min_pixelcolor/*={0,0,0,0}*/, RGBQUAD max_pixelcolor/*={255,255,255,0}*/, int accelerationmode/*=1*/)
{
	if (cell_nx > 0 && cell_ny > 0)
	{
		int nicell = cell_nx;
		int njcell = cell_ny;
		if (nicell < 1) nicell = 1;
		if (njcell < 1) njcell = 1;
		int cellwidth = width / nicell;
		int cellheight = height / njcell;
		if (cellwidth < 1) cellwidth = 1;
		if (cellheight < 1) cellheight = 1;

		int ioffset = icell * cellwidth;
		int joffset = jcell * cellheight;

		int random_integer1 = 1;
		if (pixel_inverseprobability > 1) random_integer1 = RandomInt(0, pixel_inverseprobability - 1); //1 is non-random 100%, 2 is random 50%, 3 is random 33.33%, etc.

		int cellcolor_r = pixelcolor.rgbRed; //bgcolorid_r;;
		int cellcolor_g = pixelcolor.rgbGreen; //bgcolorid_g;
		int cellcolor_b = pixelcolor.rgbBlue; //bgcolorid_b;
		if (random_integer1 == 0)
		{
			cellcolor_r = RandomInt(min_pixelcolor.rgbRed, max_pixelcolor.rgbRed); //RandomInt(idcolorlowest_r, idcolorhighest_r);
			cellcolor_g = RandomInt(min_pixelcolor.rgbGreen, max_pixelcolor.rgbGreen); //RandomInt(idcolorlowest_g, idcolorhighest_g);
			cellcolor_b = RandomInt(min_pixelcolor.rgbBlue, max_pixelcolor.rgbBlue); //RandomInt(idcolorlowest_b, idcolorhighest_b);
		}

		int random_integer2 = 1;
		if (cell_inverseprobability > 1) random_integer2 = RandomInt(0, cell_inverseprobability - 1);
		if (random_integer2 == 1)
		{
			//draw cell (sets each pixels of a cell)
			for (int i = ioffset; i < (ioffset + cellwidth); i++)
			{
				for (int j = joffset; j < (joffset + cellheight); j++)
				{
					//set pixel
					if (numbyteperpixel > 0) buffer[j*width * numbyteperpixel + i * numbyteperpixel] = cellcolor_b; //B
					if (numbyteperpixel > 1) buffer[j*width * numbyteperpixel + i * numbyteperpixel + 1] = cellcolor_g; //G
					if (numbyteperpixel > 2) buffer[j*width * numbyteperpixel + i * numbyteperpixel + 2] = cellcolor_r; //R
				}
			}
		}
		else
		{
			if (accelerationmode == 0)
			{
				//slowest, this cell could be filled with zeros
			}
			else
			{
				//don't fill this cell at all
			}
		}
		return true;
	}
	return false;
}

bool bytebuffer_cellset_setpixel(BYTE* buffer, int width, int height, int numbyteperpixel, RGBQUAD pixelcolor/*={0,0,0,0}*/, int cell_nx/*=1*/, int cell_ny/*=1*/,
	int pixel_inverseprobability/*=1*/, int cell_inverseprobability/*=1*/,
	RGBQUAD min_pixelcolor/*={0,0,0,0}*/, RGBQUAD max_pixelcolor/*={255,255,255,0}*/, int accelerationmode/*=1*/)
{
	bool result = true;
	if (buffer)
	{
		if (numbyteperpixel > 0 && numbyteperpixel < 5)
		{
			if (cell_nx > 0 && cell_ny > 0)
			{
				int nicell = cell_nx;
				int njcell = cell_ny;
				if (nicell < 1) nicell = 1;
				if (njcell < 1) njcell = 1;
				int cellwidth = width / nicell;
				int cellheight = height / njcell;
				if (cellwidth < 1) cellwidth = 1;
				if (cellheight < 1) cellheight = 1;
				for (int icell = 0; icell < nicell; icell++)
				{
					for (int jcell = 0; jcell < njcell; jcell++)
					{
						bool thisresult = bytebuffer_cell_setpixel(icell, jcell, buffer, width, height, numbyteperpixel, pixelcolor, cell_nx, cell_ny,
							pixel_inverseprobability, cell_inverseprobability,
							min_pixelcolor, max_pixelcolor, accelerationmode);
						result = result && thisresult;
					}
				}
				return result; //true only if cellset fully written
			}
		}
	}
	return false; //false when cellset NOT fully written
}

bool bytebuffer_cellset_getsetpixel(BYTE* buffer_src, BYTE* buffer_dst, int width, int height, int numbyteperpixel, RGBQUAD pixelcolor/* = { 0,0,0,0 }*/, int cell_nx/* = 1*/, int cell_ny/* = 1*/,
	int pixel_inverseprobability/* = 1*/, int cell_inverseprobability/* = 1*/,
	RGBQUAD min_pixelcolor/* = { 0,0,0,0 }*/, RGBQUAD max_pixelcolor/* = { 255,255,255,0 }*/, int accelerationmode/* = 1*/,
	float fmixratio/* = 1.0*/, float fattenuation_b/* = 1.0*/, float fattenuation_g/* = 1.0*/, float fattenuation_r/* = 1.0*/)
{
	bool result = true;
	if (buffer_dst)
	{
		if (buffer_src == NULL)
		{
			//just set
			bytebuffer_cellset_setpixel(buffer_dst, width, height, 3,
				pixelcolor,
				cell_nx, cell_ny, pixel_inverseprobability, cell_inverseprobability, //pixels and cells inverse probabilities of 2 and 2, means 1/2 and 1/2, meaning 50% and 50% chances
				min_pixelcolor, max_pixelcolor, 1);
		}
		else
		{
			//get and set
			int nicell = cell_nx;
			int njcell = cell_ny;
			if (nicell < 1) nicell = 1;
			if (njcell < 1) njcell = 1;
			int cellwidth = width / nicell;
			int cellheight = height / njcell;
			if (cellwidth < 1) cellwidth = 1;
			if (cellheight < 1) cellheight = 1;
			for (int ii = 0; ii < (fmixratio*nicell*njcell); ii++) //for (int ii = 0; ii < (fmixratio*nx*ny); ii++)
			{
				//src
				int icell = 0;
				int jcell = 0;
				if (nicell>1) icell = RandomInt(0, nicell - 1);
				if (njcell>1) jcell = RandomInt(0, njcell - 1);
				int ioffset = icell * cellwidth;
				int joffset = jcell * cellheight;
				//dst
				int iicell = 0;
				int jjcell = 0;
				if (nicell>1) iicell = RandomInt(0, nicell - 1);
				if (njcell>1) jjcell = RandomInt(0, njcell - 1);
				int iioffset = iicell * cellwidth;
				int jjoffset = jjcell * cellheight;
				//copy pixels
				for (int i = ioffset; i < (ioffset + cellwidth); i++)
				{
					for (int j = joffset; j < (joffset + cellheight); j++)
					{
						//get pixel
						//because stretch blit displaced into create canvas, optimization now requires duplicates of _dc, _bmp, _buf, and prevHGDIOBJ that are tagged _ref
						//BYTE b = _buf[j*_WIDTH * 3 + i * 3]; //B
						//BYTE g = _buf[j*_WIDTH * 3 + i * 3 + 1]; //G
						//BYTE r = _buf[j*_WIDTH * 3 + i * 3 + 2]; //R		
						BYTE b = 0;
						BYTE g = 0;
						BYTE r = 0;
						int random_integer1 = 1;
						if (pixel_inverseprobability > 1) random_integer1 = RandomInt(0, pixel_inverseprobability - 1); //1 is non-random 100%, 2 is random 50%, 3 is random 33.33%, etc.
						int cellcolor_r = pixelcolor.rgbRed; //bgcolorid_r;;
						int cellcolor_g = pixelcolor.rgbGreen; //bgcolorid_g;
						int cellcolor_b = pixelcolor.rgbBlue; //bgcolorid_b;
						if (random_integer1 == 0)
						{
							cellcolor_r = RandomInt(min_pixelcolor.rgbRed, max_pixelcolor.rgbRed); //RandomInt(idcolorlowest_r, idcolorhighest_r);
							cellcolor_g = RandomInt(min_pixelcolor.rgbGreen, max_pixelcolor.rgbGreen); //RandomInt(idcolorlowest_g, idcolorhighest_g);
							cellcolor_b = RandomInt(min_pixelcolor.rgbBlue, max_pixelcolor.rgbBlue); //RandomInt(idcolorlowest_b, idcolorhighest_b);
						}
						int random_integer2 = 1;
						if (cell_inverseprobability > 1) random_integer2 = RandomInt(0, cell_inverseprobability - 1);
						if (random_integer2 == 1)
						{
							if (numbyteperpixel > 0) b = buffer_src[j*width * numbyteperpixel + i * numbyteperpixel]; //B
							if (numbyteperpixel > 1) g = buffer_src[j*width * numbyteperpixel + i * numbyteperpixel + 1]; //G
							if (numbyteperpixel > 2) r = buffer_src[j*width * numbyteperpixel + i * numbyteperpixel + 2]; //R
						}
						//set pixel
						int iii = i - ioffset + iioffset;
						int jjj = j - joffset + jjoffset;
						if (numbyteperpixel > 0) buffer_dst[jjj*width * numbyteperpixel + iii * numbyteperpixel] = b* fattenuation_b; //B
						if (numbyteperpixel > 1) buffer_dst[jjj*width * numbyteperpixel + iii * numbyteperpixel + 1] = g* fattenuation_g; //G
						if (numbyteperpixel > 2) buffer_dst[jjj*width * numbyteperpixel + iii * numbyteperpixel + 2] = r* fattenuation_r; //R
					}
				}
			}

		}
	}
	return false;
}

//2021dec29, spi, end
