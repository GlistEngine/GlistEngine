/*
 * gWindows.cpp
 *
 *  Created on: 4 Nis 2024
 *      Author: noyan
 */

#include "gWindows.h"

gWindows::gWindows() {
	regpath = "SOFTWARE\\GlistApp\\Keys";
}

gWindows::~gWindows() {
}

void gWindows::setRegPath(std::string regPath) {
	regpath = regPath.c_str();
}

std::string gWindows::getRegPath() {
	return std::string(regpath);
}

const char* gWindows::getMachineName() {
   static char computerName[1024];
   DWORD size = 1024;
   GetComputerName( computerName, &size );
   return &(computerName[0]);
}

uint16_t gWindows::getVolumeHash() {
   DWORD serialNum = 0;

   // Determine if this volume uses an NTFS file system.
   GetVolumeInformation( "c:\\", NULL, 0, &serialNum, NULL, NULL, NULL, 0 );
   uint16_t hash = (uint16_t)(( serialNum + ( serialNum >> 16 )) & 0xFFFF );

   return hash;
}

uint16_t gWindows::getCpuHash() {
   int cpuinfo[4] = { 0, 0, 0, 0 };
   __cpuid( cpuinfo, 0 );
   uint16_t hash = 0;
   uint16_t* ptr = (uint16_t*)(&cpuinfo[0]);
   for ( uint32_t i = 0; i < 8; i++ )
      hash += ptr[i];

   return hash;
}

std::string gWindows::getMachineGUID() {
    char val[128];
    DWORD dataSize = sizeof(val);
    if(ERROR_SUCCESS == RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography", "MachineGuid", RRF_RT_ANY, nullptr, &val, &dataSize))
    	return std::string(val);
    return "0";
}

long gWindows::createRegistryKey(HKEY* hKey) {
	DWORD dwDisp = 0;
	LPDWORD lpdwDisp = &dwDisp;

	return RegCreateKeyEx(
	      HKEY_CURRENT_USER,
		  regpath,
	      0L,
	      NULL,
	      REG_OPTION_NON_VOLATILE,
	      KEY_ALL_ACCESS,
	      NULL,
	      hKey,
	      lpdwDisp);
}

long gWindows::deleteRegistryKey(HKEY hKey, std::string keyPath) {
	return RegDeleteKey(hKey, keyPath.c_str());
}

long gWindows::openRegistryKey(HKEY* hKey) {
	return RegOpenKeyEx(HKEY_CURRENT_USER,
			regpath,
	        0L,
	        KEY_ALL_ACCESS,
	        hKey);
}

long gWindows::closeRegistryKey(HKEY hKey) {
	long lReturn = RegCloseKey(hKey);
	return lReturn;
}

long gWindows::setRegistryValue(HKEY hKey, std::string name, std::string value) {
	return RegSetValueEx(hKey, name.c_str(), 0, REG_SZ, (const BYTE*)value.c_str(), value.size() + 1);
}

long gWindows::deleteRegistryValue(HKEY hKey, std::string name) {
	return RegDeleteValue(hKey, name.c_str());
}

