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
	return std::locale(locale.c_str());
}

int gToInt(const std::string& intString) {
	int x;
	std::istringstream cur(intString);
	cur >> x;
	return x;
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

