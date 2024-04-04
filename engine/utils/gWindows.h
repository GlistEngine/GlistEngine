/*
 * gWindows.h
 *
 *  Created on: 4 Nis 2024
 *      Author: noyan
 */

#ifndef UTILS_GWINDOWS_H_
#define UTILS_GWINDOWS_H_

#include "gObject.h"
#ifdef WIN32
#include <Windows.h>
#include <intrin.h>
#endif

class gWindows: public gObject {
public:
	gWindows();
	virtual ~gWindows();

#ifdef WIN32
	void setRegPath(std::string regPath);
	std::string getRegPath();

	const char* getMachineName();
	uint16_t getVolumeHash();
	uint16_t getCpuHash();
	std::string getMachineGUID();

	long createRegistryKey(HKEY* hKey);
	long deleteRegistryKey(HKEY hKey, std::string keyPath);

	long openRegistryKey(HKEY* hKey);
	long closeRegistryKey(HKEY hKey);

	long setRegistryValue(HKEY hKey, std::string name, std::string value);
	long deleteRegistryValue(HKEY hKey, std::string name);
#endif

private:
	const char* regpath;

};

#endif /* UTILS_GWINDOWS_H_ */
