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
// Only use this function for development purposes. Never use that within release build.
static std::string gGetEngineDir() {
	return "C:/dev/glist/glistengine/engine/";
}


class gObject {
public:
	static const int LOGLEVEL_SILENT, LOGLEVEL_DEBUG, LOGLEVEL_INFO, LOGLEVEL_WARNING, LOGLEVEL_ERROR;

	gObject();
	virtual ~gObject();

	/**
	 * Gives the path to the project folder.
	 *
	 *@return The location of the project folder.
     */
	static std::string gGetAppDir();

	/**
	 * Gives the path to the assets folder.
	 *
	 *@return The location of the project’s assets folder.
	 */
	static std::string gGetAssetsDir();

	/**
	 * Gives the path to the files folder.
	 *
	 *@return The location of the files folder in the project.
	 */
	static std::string gGetFilesDir();

	/**
	 * Gives the path to the image folder.
	 *
	 *@return The location of the images folder in the project.
	 */
	static std::string gGetImagesDir();

	/**
	 * Gives the path to the fonts folder.
	 *
	 *@return The location of the fonts folder in the project.
	 */
	static std::string gGetFontsDir();

	/**
	 * Gives the path to the models folder.
	 *
	 *@return The location of the models folder in the project.
	 */
	static std::string gGetModelsDir();

	/**
	 * Gives the path to the textures folder.
	 *
	 *@return The location of the textures folder in the project.
	 */
	static std::string gGetTexturesDir();

	/**
	 * Gives the path to the shaders folder.
	 *
	 *@return The location of the shaders folder in the project.
	 */
	static std::string gGetShadersDir();

	/**
	 * Gives the path to the sounds folder.
	 *
	 *@return The location of the sounds folder in the project.
	 */
	static std::string gGetSoundsDir();

	/**
	 * Gives the path to the databases folder.
	 *
	 *@return The location of the databases folder in the project.
	 */
	static std::string gGetDatabasesDir();

	/**
	 * Prints the given message to the console. It has [INFO] at the beginning.
	 * It also prints which class it is used in.
	 *
	 * @param message  The desired output. It should be a string.
	 */
	void logi(std::string message);

	/**
	 * Prints the given message to the console. It has [DEBUG] at the beginning.
	 * It also prints which class it is used in.
	 *
	 * @param message  The desired output. It should be a string.
	 */
	void logd(std::string message);

	/**
	 * Prints the given message to the console. It has [WARNING] at the beginning.
	 * It also prints which class it is used in.
	 *
	 * @param message  The desired output. It should be a string.
	 */
	void logw(std::string message);

	/**
	 * Prints the given message to the console. It has [ERROR] at the beginning.
	 * It also prints which class it is used in.
	 *  It gives yellow output.
	 *
	 * @param message  The desired output. It should be a string.
	 */
	void loge(std::string message);

	/**
	 * Prints the given message to the console. It has [INFO] at the beginning.
	 *
	 * @param tag  Reminder tag suitable for the place used. It should be a string.
	 * @param message  The desired output. It should be a string.
	 */
	void logi(std::string tag, std::string message);

	/**
	 * Prints the given message to the console. It has [DEBUG] at the beginning.
	 *
	 * @param tag  Reminder tag suitable for the place used. It should be a string.
	 * @param message  The desired output. It should be a string.
	 */
	void logd(std::string tag, std::string message);

	/**
	 * Prints the given message to the console. It has [WARNING] at the beginning.
	 *
	 * @param tag  Reminder tag suitable for the place used. It should be a string.
	 * @param message  The desired output. It should be a string.
	 */
	void logw(std::string tag, std::string message);

	/**
	 * Prints the given message to the console. It has [ERROR] at the beginning.
	 *  It gives yellow output.
	 *
	 * @param tag  Reminder tag suitable for the place used. It should be a string.
	 * @param message  The desired output. It should be a string.
	 */
	void loge(std::string tag, std::string message);

	// For internal access. Do not invoke this function directly
	static void setCurrentResolution(int scalingNo, int currentResolutionNo);

protected:
	static int renderpassnum, renderpassno;
	static int releasescaling, releaseresolution;

private:
	static std::string exepath;
};

#endif /* ENGINE_BASE_GOBJECT_H_ */
