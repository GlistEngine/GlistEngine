/*
 * gUtils.cpp
 *
 *  Created on: 5 Ara 2020
 *      Author: Acer
 */

#include "gUtils.h"
#include <sys/time.h>
#include <limits>
#include <chrono>
#include <stdio.h>
#include <ctype.h>
#include <codecvt>
#include <iterator>
#include <unistd.h>
#include <sstream>
#if defined(WIN32) || defined(LINUX) || defined(APPLE)
#include <GLFW/glfw3.h>
#endif
#if defined(ANDROID)
#include <android/log.h>
#endif

bool gLog::isloggingenabled = true;
std::string gLog::loglevelname[] = {"INFO", "DEBUG", "WARNING", "ERROR"};


int gDefaultWidth() {
	return 1280;
}

int gDefaultHeight() {
	return 720;
}

int gDefaultUnitWidth() {
	return 1280;
}

int gDefaultUnitHeight() {
	return 720;
}

int gDefaultMonitorWidth() {
#ifndef ANDROID
	int w = gDefaultWidth();
	glfwInit();
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	w = mode->width;
	glfwTerminate();
	return w;
#else
	throw std::runtime_error("not implemented!");
#endif
}

int gDefaultMonitorHeight() {
#ifndef ANDROID
	int h = gDefaultHeight();
	glfwInit();
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	h = mode->height;
	glfwTerminate();
	return h;
#else
	throw std::runtime_error("not implemented!");
#endif
}

int gDefaultScreenScaling() {
	return 2;
}

float gRadToDeg(float radians) {
	return radians * 180 / PI;
}

float gDegToRad(float degrees) {
	return degrees * PI / 180;
}

void gSeedRandom() {
	struct timeval tv;
	gettimeofday(&tv, 0);
	long int n = (tv.tv_sec ^ tv.tv_usec);
	std::srand(n);
}

float gRandom(float max) {
	return (max * rand() / float(RAND_MAX)) * (1.0f - std::numeric_limits<float>::epsilon());
}

float gRandomf() {
	return -1.0f + (2.0f * rand() / float(RAND_MAX)) * (1.0f - std::numeric_limits<float>::epsilon());
}

uint64_t gGetSystemTimeMillis() {
	struct timespec tnow;
	clock_gettime(CLOCK_MONOTONIC, &tnow);
	time_t tseconds = tnow.tv_sec;
	long tnanoseconds = tnow.tv_nsec;

	auto seconds = std::chrono::seconds(tseconds);
	auto nanoseconds = std::chrono::nanoseconds(tnanoseconds);
	return (std::chrono::duration_cast<std::chrono::milliseconds>(seconds) +
			std::chrono::duration_cast<std::chrono::milliseconds>(nanoseconds)).count();
}

unsigned int gGetUnixTime() {
	return (unsigned int)time(nullptr);
}

int gGetYear() {
	time_t curr;
	time(&curr);
	return std::localtime(&curr)->tm_year + 1900;
}

int gGetMonth() {
	time_t curr;
	time(&curr);
	return std::localtime(&curr)->tm_mon + 1;
}

int gGetDay() {
	time_t curr;
	time(&curr);
	return std::localtime(&curr)->tm_mday;
}

int gGetWeekday() {
	time_t curr;
	time(&curr);
	return std::localtime(&curr)->tm_wday;
}

int gGetHours() {
	time_t curr;
	time(&curr);
	return std::localtime(&curr)->tm_hour;
}

int gGetMinutes() {
	time_t curr;
	time(&curr);
	return std::localtime(&curr)->tm_min;
}

int gGetSeconds() {
	time_t curr;
	time(&curr);
	return std::localtime(&curr)->tm_sec;
}

//total RAM size
#ifdef WIN32
#include <windows.h>
#include <psapi.h>

uint64_t gGetTotalRamSize() {
         MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&memStatus);
        return memStatus.ullTotalPhys;
}
#elif LINUX
#include <sys/types.h>
#include <sys/sysinfo.h>

uint64_t gGetTotalRamSize() {
    struct sysinfo memInfo;
    sysinfo (&memInfo);
    long long totalPhysMem = memInfo.totalram;
    //Multiply in next statement to avoid int overflow on right hand side...
    return totalPhysMem * memInfo.mem_unit;
}
#elif APPLE
#include <sys/sysctl.h>

uint64_t gGetTotalRamSize() {
    int mib [] = { CTL_HW, HW_MEMSIZE };
    int64_t value = 0;
    size_t length = sizeof(value);

    if(-1 == sysctl(mib, 2, &value, &length, NULL, 0)) {
        // An error occurred
        return 0;
    }
    // Physical memory is now in value
    return value;
}
#endif

//Available RAM Size
#ifdef WIN32
#include <windows.h>

uint64_t gGetAvailableRamSize() {
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memStatus);
    return memStatus.ullAvailPhys;
}
#elif LINUX
#include <sys/sysinfo.h>

uint64_t gGetAvailableRamSize() {
    struct sysinfo sysInfo;
    sysinfo(&sysInfo);
    return sysInfo.freeram * sysInfo.mem_unit;
}
#elif APPLE
#include <iostream>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/mach.h>

uint64_t gGetAvailableRAMSize() {
    // Query the total physical memory size
    int mib[2];
    int64_t physicalMemorySize;
    size_t len = sizeof(physicalMemorySize);
    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    sysctl(mib, 2, &physicalMemorySize, &len, nullptr, 0);

    // Query the amount of free memory
    mach_port_t host_port = mach_host_self();
    mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
    vm_statistics64_data_t vm_stats;
    if (host_statistics64(host_port, HOST_VM_INFO, reinterpret_cast<host_info64_t>(&vm_stats), &count) != KERN_SUCCESS) {
        return 0;
    }

    uint64_t freeMemory = static_cast<uint64_t>(vm_stats.free_count) * static_cast<uint64_t>(vm_page_size);

    // Calculate available RAM size by taking the minimum of physical memory and free memory
    uint64_t availableRAM = std::min(static_cast<uint64_t>(physicalMemorySize), freeMemory);

    return availableRAM;
}
#endif

//RAM size used by GlistEngine currently
#ifdef WIN32
#include <windows.h>
#include <psapi.h>
uint64_t gGetRamSizeUsedbyGE() {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    return pmc.PrivateUsage;
}

#elif LINUX
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int parseLine(char* line){
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

uint64_t gGetRamSizeUsedbyGE() {
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmRSS:", 6) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result * 1024;
}
#elif APPLE
#include <iostream>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/mach.h>

uint64_t gGetRAMSizeUsedbyGE() {
    task_vm_info_data_t taskInfo;
    mach_msg_type_number_t taskInfoCount = TASK_VM_INFO_COUNT;
    if (task_info(mach_task_self(), TASK_VM_INFO, reinterpret_cast<task_info_t>(&taskInfo), &taskInfoCount) != KERN_SUCCESS) {
        return 0;
    }

    uint64_t residentMemory = taskInfo.phys_footprint;
    return residentMemory;
}
#endif


std::string gGetTimestampString() {
	return gGetTimestampString("%Y-%m-%d-%H-%M-%S-%i");
}

std::string gGetTimestampString(const std::string& format) {
	auto now = std::chrono::system_clock::now();
	auto timet = std::chrono::system_clock::to_time_t(now);
	std::chrono::duration<double> sc = now - std::chrono::system_clock::from_time_t(timet);
    int millis = sc.count() * 1000;
	auto timel = *std::localtime(&timet);
	char buf[256];

	auto timeformat = format;
	gStringReplace(timeformat, "%i", gToStr(millis, 3, '0'));

	std::stringstream str;
	if (strftime(buf, 256, timeformat.c_str(),&timel) != 0){
		str << buf;
	}
	auto stampstr = str.str();

    return stampstr;
}

void gStringReplace(std::string& input, const std::string& searchStr, const std::string& replaceStr) {
	auto pos = input.find(searchStr);
	while(pos != std::string::npos){
		input.replace(pos, searchStr.size(), replaceStr);
		pos += replaceStr.size();
		std::string nextfind(input.begin() + pos, input.end());
		auto nextpos = nextfind.find(searchStr);
		if(nextpos == std::string::npos){
			break;
		}
		pos += nextpos;
	}
}

std::vector<std::string> gSplitString(const std::string& textToSplit, const std::string& delimiter) {
	std::vector<std::string> tokens;
	size_t prev = 0, pos = 0;
	do {
		pos = textToSplit.find(delimiter, prev);
		if (pos == std::string::npos) pos = textToSplit.length();
		std::string token = textToSplit.substr(prev, pos - prev);
		if (!token.empty()) tokens.push_back(token);
		prev = pos + delimiter.length();
	} while (pos < textToSplit.length() && prev < textToSplit.length());
	return tokens;
}

std::string gToLower(const std::string& src, const std::string & locale) {
	std::string dst;
	std::locale loc = gGetLocale(locale);
	try{
		for(auto c: gUTF8Iterator(src)){
			utf8::append(std::tolower<wchar_t>(c, loc), back_inserter(dst));
		}
	}catch(...){
	}
	return dst;
}

std::string gToUpper(const std::string& src, const std::string & locale) {
	std::string dst;
	std::locale loc = gGetLocale(locale);
	try{
		for(auto c: gUTF8Iterator(src)){
			utf8::append(std::toupper<wchar_t>(c, loc), back_inserter(dst));
		}
	}catch(...){
	}
	return dst;
}

#ifdef WIN32
std::string gCodepointToStr(unsigned int codepoint) {
    wchar_t wideCharacter = static_cast<wchar_t>(codepoint);

    // Convert the wide character to a narrow string
    std::wstringstream wss;
    wss << wideCharacter;
    std::wstring wideString = wss.str();
    std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8Converter;
	std::string word = utf8Converter.to_bytes(wideString);
    return word;
}
#else
std::string gCodepointToStr(unsigned int codepoint) {
	std::vector<unsigned char> result;
	utf8::unchecked::utf32to8(&codepoint, &codepoint + 1, std::back_inserter(result));
	std::stringstream strs;
	for (int i = 0; i < result.size(); i++) strs << result[i];
	return strs.str();
}
#endif


static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

std::string gEncodeBase64(unsigned char* data, int len) {
	std::string ret;
	int i = 0;
	int j = 0;
	char char_array_3[3];
	char char_array_4[4];

	while (len--) {
		char_array_3[i++] = *(data++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for(i = 0; (i <4) ; i++) ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if(i) {
		for(j = i; j < 3; j++) char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++) ret += base64_chars[char_array_4[j]];

		while((i++ < 3)) ret += '=';
	}

	return ret;
}

std::string gDecodeBase64(const std::string& encodedString) {
	int in_len = encodedString.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	char char_array_4[4], char_array_3[3];
	std::vector<char> ret;

	while (in_len-- && ( encodedString[in_] != '=') && gIsBase64(encodedString[in_])) {
		char_array_4[i++] = encodedString[in_]; in_++;
		if (i ==4) {
			for (i = 0; i <4; i++) char_array_4[i] = base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++) ret.push_back(char_array_3[i]);
			i = 0;
		}
	}

	if (i) {
		for (j = i; j <4; j++) char_array_4[j] = 0;

		for (j = 0; j <4; j++) char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
	}

	return std::string(ret.begin(), ret.end());
}


bool gIsBase64(char c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}


gUTF8Iterator::gUTF8Iterator(const std::string & str){
	try{
		utf8::replace_invalid(str.begin(),str.end(),back_inserter(src_valid));
	}catch(...){
	}
}

utf8::iterator<std::string::const_iterator> gUTF8Iterator::begin() const{
	try {
		return utf8::iterator<std::string::const_iterator>(src_valid.begin(), src_valid.begin(), src_valid.end());
	}
	catch (...) {
		return utf8::iterator<std::string::const_iterator>();
	}
}

utf8::iterator<std::string::const_iterator> gUTF8Iterator::end() const{
	try {
		return utf8::iterator<std::string::const_iterator>(src_valid.end(), src_valid.begin(), src_valid.end());
	}
	catch (...) {
		return utf8::iterator<std::string::const_iterator>();
	}
}

utf8::iterator<std::string::const_reverse_iterator> gUTF8Iterator::rbegin() const {
	try {
		return utf8::iterator<std::string::const_reverse_iterator>(src_valid.rbegin(), src_valid.rbegin(), src_valid.rend());
	}
	catch (...) {
		return utf8::iterator<std::string::const_reverse_iterator>();
	}
}

utf8::iterator<std::string::const_reverse_iterator> gUTF8Iterator::rend() const {
	try {
		return utf8::iterator<std::string::const_reverse_iterator>(src_valid.rbegin(), src_valid.rbegin(), src_valid.rend());
	}
	catch (...) {
		return utf8::iterator<std::string::const_reverse_iterator>();
	}
}

std::locale gGetLocale(const std::string & locale) {
	std::locale loc;

#ifdef APPLE
	loc = std::locale(std::locale(), new std::ctype<char>);
#else
	loc = std::locale(locale.c_str());
#endif

	return loc;
}

int gToInt(const std::string& intString) {
	int x;
	std::istringstream cur(intString);
	cur >> x;
	return x;
}

float gToFloat(const std::string& floatString) {
	float f;
	std::istringstream ff(floatString);
	ff >> f;
	return f;
}

double gToDouble(const std::string& doubleString) {
	double d;
	std::istringstream dd(doubleString);
	dd >> d;
	return d;
}

std::string gWStrToStr(const std::wstring& WS) {
	const unsigned wlen = WS.length();
	char buf[wlen * sizeof(std::wstring::value_type) + 1];
	const ssize_t res = std::wcstombs(buf, WS.c_str(), sizeof(buf));
	return res != static_cast<std::size_t>(-1) ? buf : "?";
}

gLog::gLog() {
	loglevel = LOGLEVEL_INFO;
	logtag = "";
}

gLog::gLog(const std::string& tag) {
	loglevel = LOGLEVEL_INFO;
	logtag = tag;
}

gLog::~gLog() {
	if(!isloggingenabled) return;

#if ANDROID
	if(loglevel == LOGLEVEL_ERROR) {
		__android_log_print(ANDROID_LOG_ERROR, logtag.c_str(), "%s", logmessage.str().c_str());
	} else {
		__android_log_print(ANDROID_LOG_INFO, logtag.c_str(), "%s", logmessage.str().c_str());
	}
#else
	if(loglevel == LOGLEVEL_ERROR) {
		std::cerr << "[" << loglevelname[loglevel] << "] " << logtag << ": " << logmessage.str() << std::endl;
	} else {
		std::cout << "[" << loglevelname[loglevel] << "] " << logtag << ": " << logmessage.str() << std::endl;
	}
#endif
}

void gLog::setLoggingEnabled(bool isLoggingEnabled) {
	isloggingenabled = isLoggingEnabled;
}

bool gLog::isLoggingEnabled() {
	return isloggingenabled;
}

std::string gLog::getLogLevelName(int logLevel) {
	return loglevelname[logLevel];
}

gLogi::gLogi(const std::string& tag) {
	loglevel = LOGLEVEL_INFO;
	logtag = tag;
}

gLogd::gLogd(const std::string& tag) {
	loglevel = LOGLEVEL_DEBUG;
	logtag = tag;
}

gLogw::gLogw(const std::string& tag) {
	loglevel = LOGLEVEL_WARNING;
	logtag = tag;
}

gLoge::gLoge(const std::string& tag) {
	loglevel = LOGLEVEL_ERROR;
	logtag = tag;
}

void gEnableLogging() {
	gLog::setLoggingEnabled(true);
}

void gDisableLogging() {
	gLog::setLoggingEnabled(false);
}

bool gIsLoggingEnabled() {
	return gLog::isLoggingEnabled();
}

std::string gShowOpenFileDialog(
		std::string dialogTitle,
		std::string defaultPathAndFile,
	    int filterNum,
		std::string* filterPatterns,
		std::string filterDescription,
	    bool isMultipleSelectionAllowed) {
	std::string resstr = "";
	const char* carray[filterNum];
	for(int i = 0; i < filterNum; i++) carray[i] = filterPatterns[i].c_str();
	const char* res = tinyfd_openFileDialog(
			dialogTitle.c_str(),
			defaultPathAndFile.c_str(),
			filterNum,
			carray,
			filterDescription.c_str(),
			isMultipleSelectionAllowed);
	if(res) resstr = std::string(res);
	return resstr;
}

std::string gShowSaveFileDialog(
	std::string dialogTitle,
	std::string defaultPathAndFile,
    int filterNum,
	std::string* filterPatterns,
	std::string filterDescription) {
	std::string resstr = "";
	const char* carray[filterNum];
	for(int i = 0; i < filterNum; i++) carray[i] = filterPatterns[i].c_str();
	const char* res = tinyfd_saveFileDialog(
			dialogTitle.c_str(),
			defaultPathAndFile.c_str(),
			filterNum,
			carray,
			filterDescription.c_str());
	if(res) resstr = std::string(res);
	return resstr;
}

std::string gShowSelectFolderDialog(
	std::string dialogTitle,
	std::string defaultPath) {
	std::string resstr = "";
	const char* res = tinyfd_selectFolderDialog(
			dialogTitle.c_str(),
			defaultPath.c_str());
	if(res) resstr = std::string(res);
	return resstr;
}

int gShowMessageBox(
		std::string aTitle , /* NULL or "" */
		std::string aMessage , /* NULL or ""  may contain \n and \t */
		std::string aDialogType , /* "ok" "okcancel" "yesno" "yesnocancel" */
		std::string aIconType , /* "info" "warning" "error" "question" */
		int aDefaultButton ) {
	return tinyfd_messageBox(
			aTitle.c_str(),
			aMessage.c_str(),
			aDialogType.c_str(),
			aIconType.c_str(),
			aDefaultButton);
}

std::string gShowInputBox(
		std::string aTitle , /* NULL or "" */
		std::string aMessage , /* NULL or ""  may contain \n and \t */
		std::string aDefaultInput) /* "" , if NULL it's a passwordBox */ {
	std::string resstr = "";
	const char* res = tinyfd_inputBox(
			aTitle.c_str(),
			aMessage.c_str(),
			aDefaultInput.c_str());
	if(res) resstr = std::string(res);
	return resstr;
}

gColor gShowColorChooser(
	std::string aTitle , /* "" */
	std::string aDefaultHexRGB , /* NULL or "#FF0000" */
	unsigned char const aDefaultRGB[3]) /* { 0 , 255 , 255 } */ {
	gColor selectedcolor(0, 0, 0, 255);
	unsigned char result[3];
	char const * res = tinyfd_colorChooser(aTitle.c_str(), aDefaultHexRGB.c_str(), aDefaultRGB, result);
	if(res) {
		selectedcolor.set(result[0] / 255, result[1] / 255, result[2] / 255);
	}
	return selectedcolor;
}

bool gCheckCollision(int xLeft1, int yUp1, int xRight1, int yBottom1,
		int xLeft2, int yUp2, int xRight2, int yBottom2) {
	return xLeft1 < xRight2 && xRight1 > xLeft2 && yBottom1 > yUp2 && yUp1 < yBottom2;
}

gUtils::gUtils() {

}

gUtils::~gUtils() {
}

