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
	static void setRegPath(std::string regPath);
	static std::string getRegPath();

	static const char* getMachineName();
	static uint16_t getVolumeHash();
	static uint16_t getCpuHash();
	static std::string getMachineGUID();

	static long createRegistryKey(HKEY* hKey, std::string regPath = regpath, HKEY registryHive = HKEY_CURRENT_USER);
	static long deleteRegistryKey(HKEY hKey, std::string keyPath);

	static long openRegistryKey(HKEY* hKey, std::string regPath = regpath, HKEY registryHive = HKEY_CURRENT_USER);
	static long closeRegistryKey(HKEY hKey);

	static long setRegistryValue(HKEY hKey, std::string name, std::string value);
	static std::string getRegistryValue(std::string name, std::string regPath = regpath, HKEY registryHive = HKEY_CURRENT_USER);
	static long deleteRegistryValue(HKEY hKey, std::string name);
#endif

private:
	static std::string regpath;

};

#endif /* UTILS_GWINDOWS_H_ */
