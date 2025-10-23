/*
 * gUtils.h
 *
 *  Created on: 5 Ara 2020
 *      Author: Noyan
 */

#pragma once

#ifndef UTILS_GUTILS_H_
#define UTILS_GUTILS_H_

#include <string>
#include <sstream>
#include <ostream>
#include <iostream>
#include <iomanip>
#include "utf8.h"
#include <stdlib.h>
#include <wchar.h>
#include <vector>
#include "gConstants.h"
#include "gKeyCode.h"
#include "gGUIEvents.h"
#include "tinyfiledialogs.h"
#include "gColor.h"
#include <cstdint>

/*
#ifndef LINUX
	#define LINUX __linux__
#endif
*/
#ifdef _WIN64
   //define something for Windows (64-bit)
	#ifndef WIN64
		#define WIN64 _WIN32
	#endif
#elif _WIN32
   //define something for Windows (32-bit)
	#ifndef WIN32
		#define WIN32 _WIN32
	#endif
#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_IPHONE_SIMULATOR
        // define something for simulator
    #elif TARGET_OS_IPHONE
        // define something for iphone
    #else
		#ifndef TARGET_OS_OSX
			#define TARGET_OS_OSX 1
		#endif
		#ifndef APPLE
			#define APPLE __APPLE__
		#endif
        // define something for OSX
    #endif
#elif __ANDROID__
    // Android
#elif __linux
    // Linux
	#ifndef LINUX
		#define LINUX __linux__
	#endif
#elif __unix // all unices not caught above
    // Unix
#elif __posix
    // POSIX
#endif

#if ANDROID || TARGET_OS_IPHONE || TARGET_OS_SIMULATOR || EMSCRIPTEN
#ifndef GLIST_OPENGLES
#define GLIST_OPENGLES 1
#endif
#endif

#if EMSCRIPTEN
#ifndef GLIST_WEB
#define GLIST_WEB 1
#endif
#endif
#if TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
#ifndef GLIST_IOS
#define GLIST_IOS 1
#endif
#endif
#if ANDROID
#ifndef GLIST_ANDROID
#define GLIST_ANDROID 1
#endif
#endif


#ifndef PI
	#define PI       3.14159265358979323846
#endif

class gImage;
int gDefaultWidth();
int gDefaultHeight();
int gDefaultUnitWidth();
int gDefaultUnitHeight();
int gDefaultMonitorWidth();
int gDefaultMonitorHeight();
int gDefaultScreenScaling();

float gRadToDeg(float radians);
float gDegToRad(float degrees);

void gSeedRandom();
float gRandom(float max);
float gRandomf();

uint64_t gGetSystemTimeMillis();
unsigned int gGetUnixTime();

/*
 * Returns YYYY
 */
int gGetYear();

/*
 * Returns M, starting from 1
 */
int gGetMonth();

/*
 * Returns D, starting from 1
 */
int gGetDay();

int gGetWeekday();
int gGetHours();
int gGetMinutes();
int gGetSeconds();


#if  defined(WIN32) || defined(LINUX) || defined(APPLE)    //Available Ram Size
uint64_t gGetAvailableRamSize();
#endif

#if  defined(WIN32) || defined(LINUX) || defined(APPLE)     //Current Ram Size Used by Glist Engine
uint64_t gGetRamSizeUsedbyGE();
#endif

#if  defined(WIN32) || defined(LINUX) || defined(APPLE)    //Total Ram Size
uint64_t gGetTotalRamSize();
#endif

/*
 * Rotates The Pixel Data upside down. Hence rotates flips the image upside down
 */
void gFlipImageDataVertically(unsigned char* pixelData, int width, int height, int numChannels);

std::string gGetTimestampString();
std::string gGetTimestampString(const std::string& format);
void gStringReplace(std::string& input, const std::string& searchStr, const std::string& replaceStr);
std::locale gGetLocale(const std::string & locale);
std::string gToLower(const std::string& src, const std::string & locale = "");
std::string gToUpper(const std::string& src, const std::string & locale = "");
std::vector<std::string> gSplitString(const std::string& textToSplit, const std::string& delimiter);
std::string gReplaceAll(const std::string& source, const std::string& from, const std::string& to);
bool gIsValidFilename(std::string fileName);

template <class T>
std::string gToStr(const T& numValue) {
	std::ostringstream out;
	out << numValue;
	return out.str();
}

template <class T>
std::string gToHex(const T& numValue, int width = sizeof(T)*2) {
	std::ostringstream stream;
	stream << "0x"
		   << std::setfill ('0') << std::setw(width)
		   << std::hex << numValue;
	return stream.str();
}

template <class T>
std::string gToStr(const T& value, int width, char fill) {
	std::ostringstream out;
	out << std::fixed << std::setfill(fill) << std::setw(width) << value;
	return out.str();
}

int gToInt(const std::string& intString);
float gToFloat(const std::string& floatString);
double gToDouble(const std::string& doubleString);
std::string gWStrToStr(const std::wstring& s);

template <typename T>
int gSign(T val) {
    return (T(0) < val) - (val < T(0));
}

std::string gCodepointToStr(unsigned int codepoint);

std::string gEncodeBase64(unsigned char* data, int len);
std::string gDecodeBase64(const std::string& encoded_string);

bool gIsBase64(char c);

bool gIsOnline();
void gOpenUrlInDefaultBrowser(std::string url);

class gUTF8Iterator{
public:
	gUTF8Iterator(const std::string & str);
	utf8::iterator<std::string::const_iterator> begin() const;
	utf8::iterator<std::string::const_iterator> end() const;
	utf8::iterator<std::string::const_reverse_iterator> rbegin() const;
	utf8::iterator<std::string::const_reverse_iterator> rend() const;

private:
	std::string src_valid;
};

class gLog {
public:
	enum {
		LOGLEVEL_INFO,
		LOGLEVEL_DEBUG,
		LOGLEVEL_WARNING,
		LOGLEVEL_ERROR
	};

	gLog();
	gLog(const std::string& tag);
	~gLog();

	gLog& operator<<(std::ostream& (*func)(std::ostream&)){
		func(logmessage);
		return *this;
	}

	template <typename T>
	gLog& operator<<(const T& value) {
		logmessage << value;
		return *this;
	}

	static void setLoggingEnabled(bool isLoggingEnabled);
	static bool isLoggingEnabled();

	static std::string getLogLevelName(int logLevel);

protected:
	static bool isloggingenabled;
	int loglevel;
	std::stringstream logmessage;
	std::string logtag;
	static std::string loglevelname[];

private:
	std::string logmsg;
};

class gLogi : public gLog {
public:
	gLogi(const std::string& tag = "");
};

class gLogd : public gLog {
public:
	gLogd(const std::string& tag = "");
};

class gLogw : public gLog {
public:
	gLogw(const std::string& tag = "");
};

class gLoge : public gLog {
public:
	gLoge(const std::string& tag = "");
};

void gEnableLogging();
void gDisableLogging();
bool gIsLoggingEnabled();

template<typename T>
T gClamp(T t, T min, T max) {
	return t > max ? max : t < min ? min : t;
}

/**
 * Shows operating system's open file dialog
 *
 * EXAMPLE USAGE
 * ```cpp
 * // GameCanvas.h:
 * static const int patternnum = 2;
 * std::string filterpatterns[patternnum];
 * std::string filepath;
 *
 * // GameCanvas.cpp:
 * void GameCanvas::setup() {
 *     filterpatterns[0] = "*.csv";
 *     filterpatterns[1] = "*.txt";
 *     filepath = "";
 * }
 *
 * void GameCanvas::mouseReleased(int x, int y, int button) {
 *     std::string openfilename = gShowOpenFileDialog("Open File", gGetFilesDir(), patternnum, filterpatterns, "Data files", false);
 *     if(!openfilename.empty()) {
 *         filepath = openfilename;
 *		   gLogi("GameCanvas") << "filepath:" << filepath;
 *     } else {
 *		   gLogi("GameCanvas") << "No file selected!";
 *     }
 * }
 * ```
 *
 * @param dialogTitle Dialog's title
 * @param defaultPathAndFile Default path to show
 * @param filterNum Filter pattern number
 * @param filterPatterns A string array of the file patterns to browse
 * @param filterDescription Description of the filter such as 'Image files'
 * @param isMultipleSelectionAllowed Is multiple selection allowed. Default value is false.
 * @return Chosen file's full path
 */
std::string gShowOpenFileDialog(
	const std::string& dialogTitle,
	const std::string& defaultPathAndFile,
    int filterNum,
	const std::string* filterPatterns,
	const std::string& filterDescription,
    bool isMultipleSelectionAllowed = false);

/**
 * Shows the operating system's open file dialog
 *
 * EXAMPLE USAGE
 * ```cpp
 * std::string filepath = gShowOpenFileDialog(
 *     "Open File",
 *     gGetFilesDir(),
 *     {"*.csv", "*.txt"},
 *     "Data files",
 *     false
 * );
 * if(!filepath.empty()) {
 *     gLogi("FileDialog") << "Selected file: " << filepath;
 * }
 * ```
 *
 * @param dialogTitle Dialog's title.
 * @param defaultPathAndFile Default path and file to display in the dialog.
 * @param filterPatterns An initializer list of file patterns to filter (e.g., {"*.txt", "*.csv"}).
 * @param filterDescription Description of the file filter (e.g., "Text files").
 * @param isMultipleSelectionAllowed If true, allows multiple file selection. Default is false.
 * @return The full path of the selected file as a string. Returns an empty string if no file is selected.
 */
std::string gShowOpenFileDialog(
	const std::string& dialogTitle,
	const std::string& defaultPathAndFile,
	std::initializer_list<const char*> filterPatterns,
	const std::string& filterDescription,
	bool isMultipleSelectionAllowed = false);

std::string gShowOpenFileDialog(
	const std::string& dialogTitle,
	const std::string& defaultPathAndFile,
	int filterNum,
	const char** filterPatterns,
	const std::string& filterDescription,
	bool isMultipleSelectionAllowed = false);

std::string gShowSaveFileDialog(
	const std::string& dialogTitle,
	const std::string& defaultPathAndFile,
    int filterNum,
	const std::string* filterPatterns,
	const std::string& filterDescription);


std::string gShowSaveFileDialog(
	const std::string& dialogTitle,
	const std::string& defaultPathAndFile,
	std::initializer_list<const char*> filterPatterns,
	const std::string& filterDescription);

std::string gShowSaveFileDialog(
	const std::string& dialogTitle,
	const std::string& defaultPathAndFile,
	int filterNum,
	const char** filterPatterns,
	const std::string& filterDescription);

std::string gShowSelectFolderDialog(
	const std::string& dialogTitle,
	const std::string& defaultPath);


/**
 * Shows operating system's message dialog box
 *
 * @param title Dialog's title
 * @param message Dialog's message text. May contain \n and \t or be empty
 * @param dialogue Dialog's type. Possible values are "ok", "okcancel", "yesno", "yesnocancel"
 * @param iconType Dialog's type. Possible values are "info", "warning", "error", "question"
 * @return clicked button
 */
int gShowMessageBox(
	const std::string& title,
	const std::string& message,
	const std::string& dialogue,
	const std::string& iconType,
	int defaultButton);

/**
 * Shows operating system's input dialog box
 *
 * @param title Dialog's title. Can be empty.
 * @param message Dialog's message text. May contain \n and \t or be empty.
 * @param defaultInput Default input text. If empty, it will display a password box.
 * @return User input as a string.
 */
std::string gShowInputBox(
		const std::string& title,
		const std::string& message,
		const std::string& defaultInput);

/**
 * Shows operating system's color chooser dialog box
 *
 * @param title Dialog's title. Can be empty.
 * @param defaultHex Default color in hexadecimal format (e.g., "#FF0000"). Can be empty.
 * @param defaultRGB Default color in RGB format as an array of three unsigned char values (e.g., {0, 255, 255}).
 * @return Selected color as a gColor object.
 */
gColor gShowColorChooser(
	const std::string& title,
	const std::string& defaultHex,
	unsigned char const defaultRGB[3]);

bool gCheckCollision(int xLeft1, int yUp1, int xRight1, int yBottom1,
		int xLeft2, int yUp2, int xRight2, int yBottom2);

bool gCheckPixelPerfectCollision(gImage* image1, int x1, int y1, gImage* image2, int x2, int y2);

std::string gFixPath(const std::string& path);

class gUtils {
public:
	gUtils();
	virtual ~gUtils();
};

#endif /* UTILS_GUTILS_H_ */
