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
#include <iterator>
#include <unistd.h>

bool gLog::isloggingenabled = true;
std::string gLog::loglevelname[] = {"INFO", "DEBUG", "WARNING", "ERROR"};


int gDefaultWidth() {
	return 960;
}

int gDefaultHeight() {
	return 540;
}

int gDefaultUnitWidth() {
	return 960;
}

int gDefaultUnitHeight() {
	return 540;
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
	long int n = (tv.tv_sec ^ tv.tv_usec) ^ getpid();
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
	char c;
	wchar_t ch = (wchar_t)codepoint;
	std::wctomb(&c, ch);
	return std::string(1, c);
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


std::string gEncodeBase64(unsigned const char* data, int len) {
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

	if(loglevel == LOGLEVEL_ERROR) {
		std::cerr << "[" << loglevelname[loglevel] << "] " << logtag << ": " << logmessage.str() << std::endl;
	} else {
		std::cout << "[" << loglevelname[loglevel] << "] " << logtag << ": " << logmessage.str() << std::endl;
	}
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

gUtils::gUtils() {

}

gUtils::~gUtils() {
}

