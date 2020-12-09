/*
 * gObject.h
 *
 *  Created on: May 16, 2020
 *      Author: noyan
 */

#ifndef ENGINE_BASE_GOBJECT_H_
#define ENGINE_BASE_GOBJECT_H_

#include "gUtils.h"
#include <cxxabi.h>
//#include "gConfig.h"


// TODO Change hardcoded engine path
// Only use this variable for development purposes. Never export within release build.
static std::string gGetEngineDir() {
	return "C:/dev/glist/glistengine/engine/";
}


class gObject {
public:
	static const int LOGLEVEL_SILENT, LOGLEVEL_DEBUG, LOGLEVEL_INFO, LOGLEVEL_WARNING, LOGLEVEL_ERROR;

	gObject();
	virtual ~gObject();

	std::string gGetAppDir();
	std::string gGetAssetsDir();
	std::string gGetImagesDir();
	std::string gGetFontsDir();
	std::string gGetModelsDir();
	std::string gGetSoundsDir();

	void logi(std::string message);
	void logd(std::string message);
	void logw(std::string message);
	void loge(std::string message);

	void logi(std::string tag, std::string message);
	void logd(std::string tag, std::string message);
	void logw(std::string tag, std::string message);
	void loge(std::string tag, std::string message);

	void setLogLevel(int logLevel);


private:
	int loglevel;
	std::string loglevelname[5];
	void log(int logLevel, std::string tag, std::string message);
	std::string exepath;
};

#endif /* ENGINE_BASE_GOBJECT_H_ */
