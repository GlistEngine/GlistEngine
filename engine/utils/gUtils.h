/*
 * gUtils.h
 *
 *  Created on: 5 Ara 2020
 *      Author: Acer
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


#ifndef PI
	#define PI       3.14159265358979323846
#endif

int gDefaultWidth();
int gDefaultHeight();
int gDefaultUnitWidth();
int gDefaultUnitHeight();
int gDefaultScreenScaling();

float gRadToDeg(float radians);
float gDegToRad(float degrees);

void gSeedRandom();
float gRandom(float max);
float gRandomf();

uint64_t gGetSystemTimeMillis();
unsigned int gGetUnixTime();
int gGetYear();
int gGetMonth();
int gGetDay();
int gGetWeekday();
int gGetHours();
int gGetMinutes();
int gGetSeconds();
std::string gGetTimestampString();
std::string gGetTimestampString(const std::string& format);
void gStringReplace(std::string& input, const std::string& searchStr, const std::string& replaceStr);
std::locale gGetLocale(const std::string & locale);
std::string gToLower(const std::string& src, const std::string & locale = "");
std::string gToUpper(const std::string& src, const std::string & locale = "");
std::vector<std::string> gSplitString(const std::string& textToSplit, const std::string& delimiter);

template <class T>
std::string gToStr(const T& numValue) {
	std::ostringstream out;
	out << numValue;
	return out.str();
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

std::string gEncodeBase64(unsigned const char* input, int len);
std::string gDecodeBase64(const std::string& encoded_string);
bool gIsBase64(char c);

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

/**
 * Shows operating system's open file dialog
 *
 * EXAMPLE USAGE
 * GameCanvas.h:
 * bool showopenfiledialog;
 * static const int patternnum = 2;
 * std::string filterpatterns[patternnum];
 * std::string openfilename;
 * std::string filepath;
 *
 * GameCanvas.cpp:
 * void GameCanvas::setup() {
 *     showopenfiledialog = false;
 *     filterpatterns[0] = "*.csv";
 *     filterpatterns[1] = "*.txt";
 *     openfilename = "";
 *     filepath = "";
 * }
 *
 * void GameCanvas::update() {
 *     if(showopenfiledialog) {
 *         openfilename = gShowOpenFileDialog("Open File", gGetFilesDir(), patternnum, filterpatterns, "Data files", false);
 *         if(openfilename != "") filepath = openfilename;
 *         openfilename = "";
 *         showopenfiledialog = false;
 *     }
 *     gLogi("GameCanvas") << "filepath:" << filepath;
 * }
 *
 * void GameCanvas::mouseReleased(int x, int y, int button) {
 *     showopenfiledialog = true;
 * }
 *
 *
 * @param dialogTitle Dialog's title
 * @param defaultPathAndFile Default path to show
 * @param filterNum Filter pattern number
 * @param filterPatterns A string array of the file patterns to browse
 * @param filterDescription Description of the filter such as 'Image files'
 * @param isMultipleSelectionAllowed Is multiple selection allowed. Default value is false.
 * @return Choosen file's full path
 */
std::string gShowOpenFileDialog(
	std::string dialogTitle,
	std::string defaultPathAndFile,
    int filterNum,
	std::string* filterPatterns,
	std::string filterDescription,
    bool isMultipleSelectionAllowed = false);


/**
 * Shows operating system's message dialog box
 *
 * @param aTitle Dialog's title
 * @param aMessage Dialog's message text. May contain \n and \t
 * @param aDialogType Dialog's type. Possible values are "ok", "okcancel", "yesno", "yesnocancel"
 * @param aIconType Dialog's type. Possible values are "info", "warning", "error", "question"
 * @return clicked button
 */
int gShowMessageBox(
	std::string aTitle , /* NULL or "" */
	std::string aMessage , /* NULL or ""  may contain \n and \t */
	std::string aDialogType , /* "ok" "okcancel" "yesno" "yesnocancel" */
	std::string aIconType , /* "info" "warning" "error" "question" */
	int aDefaultButton ) /* 0 for cancel/no , 1 for ok/yes , 2 for no in yesnocancel */;

class gUtils {
public:
	gUtils();
	virtual ~gUtils();
};

#endif /* UTILS_GUTILS_H_ */
