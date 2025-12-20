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
bool gObject::initializedpaths = false;


static const std::string resolutiondirs[] = {
		"1-8k/", "2-4k/", "3-qhd/", "4-fullhd/", "5-hd/", "6-qfhd/", "7-wvga/", "8-hvga/"
};

int gObject::renderpassnum = 1;
int gObject::renderpassno = 0;

inline bool endsWith(std::string const & value, std::string const & ending) {
	if (ending.size() > value.size()) {
		return false;
	}
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

gObject::gObject() {
}

std::string gObject::gGetAppDir() {
	initPaths();
	return exepath;
}

std::string gObject::gGetAssetsDir() {
	initPaths();
	return assetsdir;
}

void gObject::gSetAssetsDir(std::string assetsDir) {
	initPaths();
	gObject::assetsdir = assetsDir;
}

std::string gObject::gGetFilesDir() {
	return gGetAssetsDir() + "files/";
}

std::string gObject::gGetImagesDir() {
	if (releasescaling == G_SCREENSCALING_MIPMAP) return gGetAssetsDir() + "mipmaps/" + resolutiondirs[releaseresolution];
	return gGetAssetsDir() + "images/";
}

std::string gObject::gGetFontsDir() {
	return gGetAssetsDir() + "fonts/";
}

std::string gObject::gGetModelsDir() {
	return gGetAssetsDir() + "models/";
}

std::string gObject::gGetTexturesDir() {
	return gGetAssetsDir() + "textures/";
}

std::string gObject::gGetShadersDir() {
	return gGetAssetsDir() + "shaders/";
}

std::string gObject::gGetSoundsDir() {
	return gGetAssetsDir() + "sounds/";
}

std::string gObject::gGetDatabasesDir() {
	return gGetAssetsDir() + "databases/";
}

std::string gObject::gGetVideosDir() {
	return gGetAssetsDir() + "videos/";
}

void gObject::setCurrentResolution(int scalingNo, int currentResolutionNo) {
	releasescaling = scalingNo;
	releaseresolution = currentResolutionNo;
}

void gObject::initPaths() {
	if (initializedpaths) {
		return;
	}
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
	if(assetsdir.empty()) {
		assetsdir = exepath + "assets/";
	}
#endif
	initializedpaths = true;
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
