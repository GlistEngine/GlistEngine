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


#ifndef LINUX
	#define LINUX __linux__
#endif


#ifndef PI
	#define PI       3.14159265358979323846
#endif

int gDefaultWidth();
int gDefaultHeight();

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

template <typename T>
int gSign(T val) {
    return (T(0) < val) - (val < T(0));
}

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


class gUtils {
public:
	gUtils();
	virtual ~gUtils();
};

#endif /* UTILS_GUTILS_H_ */
