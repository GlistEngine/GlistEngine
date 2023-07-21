/*
* gAndroidUtil.h
*
*  Created on: June 24, 2023
*      Author: Metehan Gezer
*/

#ifndef GANDROIDUTIL_H
#define GANDROIDUTIL_H

#ifdef ANDROID

#include <android/log.h>
#include <android/native_window.h> // requires ndk r5 or newer
#include <android/asset_manager_jni.h>
#include <android/asset_manager.h>
#include <string>

void androidMain();

class gAndroidUtil {
public:
    static AAssetManager* assets;

    static AAsset* loadAsset(const std::string& path, int mode);
    static void closeAsset(AAsset* asset);
};
#endif /* ANDROID */

#endif //GANDROIDUTIL_H
