/*
 * Copyright (C) 2016 Nitra Games Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

	static std::string gGetAppDir();
	static std::string gGetAssetsDir();
	static std::string gGetFilesDir();
	static std::string gGetImagesDir();
	static std::string gGetFontsDir();
	static std::string gGetModelsDir();
	static std::string gGetTexturesDir();
	static std::string gGetShadersDir();
	static std::string gGetSoundsDir();

	void logi(std::string message);
	void logd(std::string message);
	void logw(std::string message);
	void loge(std::string message);

	void logi(std::string tag, std::string message);
	void logd(std::string tag, std::string message);
	void logw(std::string tag, std::string message);
	void loge(std::string tag, std::string message);

protected:
	static int renderpassnum, renderpassno;

private:
	static std::string exepath;
};

#endif /* ENGINE_BASE_GOBJECT_H_ */
