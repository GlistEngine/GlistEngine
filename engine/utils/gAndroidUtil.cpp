//
// Created by Metehan Gezer on 24.06.2023.
//

#include "gAndroidUtil.h"

AAssetManager* gAndroidUtil::assets;

AAsset* gAndroidUtil::loadAsset(const std::string &path, int mode) {
    return AAssetManager_open(assets, path.c_str(), mode);
}

void gAndroidUtil::closeAsset(AAsset *asset) {
    AAsset_close(asset);
}

extern "C" {
JNIEXPORT void JNICALL Java_dev_glist_android_lib_GlistNative_setAssetManager(JNIEnv *env, jclass clazz, jobject assets) {
    AAssetManager* man = AAssetManager_fromJava(env, assets);
    gAndroidUtil::assets = man;
}
}
