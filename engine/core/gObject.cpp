/*
 * gObject.cpp
 *
 *  Created on: May 16, 2020
 *      Author: noyan
 */

#include "gObject.h"
#include <unistd.h>

const int gObject::LOGLEVEL_SILENT = 0;
const int gObject::LOGLEVEL_INFO = 1;
const int gObject::LOGLEVEL_DEBUG = 2;
const int gObject::LOGLEVEL_WARNING = 3;
const int gObject::LOGLEVEL_ERROR = 4;


gObject::gObject() {
	loglevelname[0] = "Silent";
	loglevelname[1] = "Info";
	loglevelname[2] = "Debug";
	loglevelname[3] = "Warning";
	loglevelname[4] = "Error";
	loglevel = LOGLEVEL_INFO;
	char temp[256];
	exepath = getcwd(temp, sizeof(temp));
	exepath += "/";
	for (int i = 0; i < exepath.size(); i++) {
	    if (exepath[i] == '\\') {
	    	exepath[i] = '/';
	    }
	}
//	std::replace(0, 1, "", "");
}

gObject::~gObject() {
}

const std::string& gObject::gGetAssetsDir() {

}
const std::string& gObject::gGetImagesDir() {
	return exepath + "assets/images/";
}

const std::string& gObject::gGetFontsDir() {
	return exepath + "assets/fonts/";
}

const std::string& gObject::gGetModelsDir() {
	return exepath + "assets/models/";
}

const std::string& gObject::gGetSoundsDir() {
	return exepath + "assets/sounds/";
}

void gObject::setLogLevel(int logLevel) {
	loglevel = logLevel;
}


void gObject::log(int logLevel, const std::string& tag, const std::string& message) {
	std::cout << "[" << loglevelname[logLevel] << "] " << tag << ": " << message << std::endl;
}


void gObject::logi(const std::string& tag, const std::string& message) {
	if (loglevel < LOGLEVEL_INFO) return;
	log(LOGLEVEL_INFO, tag, message);
}

void gObject::logi(const std::string& message) {
	if (loglevel < LOGLEVEL_INFO) return;
	log(LOGLEVEL_INFO, abi::__cxa_demangle(typeid(*this).name(), 0, 0, 0), message);
}

void gObject::logd(const std::string& tag, const std::string& message) {
	if (loglevel < LOGLEVEL_DEBUG) return;
	log(LOGLEVEL_DEBUG, tag, message);
}

void gObject::logd(const std::string& message) {
	if (loglevel < LOGLEVEL_DEBUG) return;
	log(LOGLEVEL_DEBUG, abi::__cxa_demangle(typeid(*this).name(), 0, 0, 0), message);
}

void gObject::logw(const std::string& tag, const std::string& message) {
	if (loglevel < LOGLEVEL_WARNING) return;
	log(LOGLEVEL_WARNING, tag, message);
}

void gObject::logw(const std::string& message) {
	if (loglevel < LOGLEVEL_WARNING) return;
	log(LOGLEVEL_WARNING, abi::__cxa_demangle(typeid(*this).name(), 0, 0, 0), message);
}

void gObject::loge(const std::string& tag, const std::string& message) {
	if (loglevel < LOGLEVEL_ERROR) return;
	std::cerr << "[" << loglevelname[LOGLEVEL_ERROR] << "] " << tag << ": " << message << std::endl;
}

void gObject::loge(const std::string& message) {
	if (loglevel < LOGLEVEL_ERROR) return;
	loge(abi::__cxa_demangle(typeid(*this).name(), 0, 0, 0), message);
}

