/*
 * gObject.cpp
 *
 *  Created on: May 16, 2020
 *      Author: noyan
 */

#include "gObject.h"
#include <unistd.h>
#if defined(ANDROID)
#include "gAndroidUtil.h"
#elif TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
#include "gIOSUtil.h"
#endif

const int gObject::LOGLEVEL_SILENT = 0;
const int gObject::LOGLEVEL_INFO = 1;
const int gObject::LOGLEVEL_DEBUG = 2;
const int gObject::LOGLEVEL_WARNING = 3;
const int gObject::LOGLEVEL_ERROR = 4;

int gObject::releasescaling;
int gObject::releaseresolution;

std::string gObject::exepath;
std::string gObject::assetsdir;


static const std::string resolutiondirs[] = {
		"1-8k/", "2-4k/", "3-qhd/", "4-fullhd/", "5-hd/", "6-qfhd/", "7-wvga/", "8-hvga/"
};

int gObject::renderpassnum = 1;
int gObject::renderpassno = 0;

inline bool endsWith(std::string const & value, std::string const & ending)
{
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

gObject::gObject() {
	char temp[256];
	exepath = getcwd(temp, sizeof(temp));
	if(!endsWith(exepath, "/")) {
		exepath += "/";
	}

	for (int i = 0; i < exepath.size(); i++) {
	    if (exepath[i] == '\\') {
	    	exepath[i] = '/';
	    }
	}
#if defined(ANDROID)
    if(gAndroidUtil::datadirectory.empty()) {
        assetsdir = "";
    } else {
        assetsdir = gAndroidUtil::datadirectory + "/";
    }
#elif TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
    assetsdir = gGetIOSResourceDirectory() + "/";
#else
    if(assetsdir == "") {
        assetsdir = exepath + "assets/";
    }
#endif
}

std::string gObject::gGetAppDir() {
	return exepath;
}

std::string gObject::gGetAssetsDir() {
	return assetsdir;
}

void gObject::gSetAssetsDir(std::string assetsDir) {
	gObject::assetsdir = assetsDir;
}

std::string gObject::gGetFilesDir() {
	return assetsdir + "files/";
}

std::string gObject::gGetImagesDir() {
	if (releasescaling == 1) return assetsdir + "mipmaps/" + resolutiondirs[releaseresolution];
	return assetsdir + "images/";
}

std::string gObject::gGetFontsDir() {
	return assetsdir + "fonts/";
}

std::string gObject::gGetModelsDir() {
	return assetsdir + "models/";
}

std::string gObject::gGetTexturesDir() {
	return assetsdir + "textures/";
}

std::string gObject::gGetShadersDir() {
	return assetsdir + "shaders/";
}

std::string gObject::gGetSoundsDir() {
	return assetsdir + "sounds/";
}

std::string gObject::gGetDatabasesDir() {
	return assetsdir + "databases/";
}

std::string gObject::gGetVideosDir() {
	return assetsdir + "videos/";
}

void gObject::setCurrentResolution(int scalingNo, int currentResolutionNo) {
	releasescaling = scalingNo;
	releaseresolution = currentResolutionNo;
}


void gObject::logi(std::string tag, std::string message) {
	if(!gIsLoggingEnabled()) return;
	gLogi(tag) << message;
}

void gObject::logi(std::string message) {
	if(!gIsLoggingEnabled()) return;
	gLogi(abi::__cxa_demangle(typeid(*this).name(), 0, 0, 0)) << message;
}

void gObject::logd(std::string tag, std::string message) {
	if(!gIsLoggingEnabled()) return;
	gLogd(tag) << message;
}

void gObject::logd(std::string message) {
	if(!gIsLoggingEnabled()) return;
	gLogd(abi::__cxa_demangle(typeid(*this).name(), 0, 0, 0)) << message;
}

void gObject::logw(std::string tag, std::string message) {
	if(!gIsLoggingEnabled()) return;
	gLogw(tag) << message;
}

void gObject::logw(std::string message) {
	if(!gIsLoggingEnabled()) return;
	gLogw(abi::__cxa_demangle(typeid(*this).name(), 0, 0, 0)) << message;
}

void gObject::loge(std::string tag, std::string message) {
	if(!gIsLoggingEnabled()) return;
	gLoge(tag) << message;
}

void gObject::loge(std::string message) {
	if(!gIsLoggingEnabled()) return;
	gLoge(abi::__cxa_demangle(typeid(*this).name(), 0, 0, 0)) << message;
}
