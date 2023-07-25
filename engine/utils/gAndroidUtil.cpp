/*
 * gAndroidUtil.cpp
 *
 *  Created on: June 24, 2023
 *      Author: Metehan Gezer
 */


#include <gUtils.h>
#include "gAndroidUtil.h"

AAssetManager* gAndroidUtil::assets;
JavaVM* javavm;

AAsset* gAndroidUtil::loadAsset(const std::string &path, int mode) {
	return AAssetManager_open(assets, path.c_str(), mode);
}

void gAndroidUtil::closeAsset(AAsset *asset) {
	AAsset_close(asset);
}

jmethodID gAndroidUtil::getJavaMethodID(jclass classID, std::string methodName, std::string methodSignature) {
	jmethodID result = getJNIEnv()->GetMethodID(classID, methodName.c_str(), methodSignature.c_str());

	if(!result) {
		gLoge("gAndroidUtil") << "couldn't find instance method '"
							  << methodName << "' with signature '"
							  << methodSignature << "' in class '"
							  << getJavaClassName(classID) << "'";
		return nullptr;
	}

	return result;
}

jmethodID gAndroidUtil::getJavaStaticMethodID(jclass classID, std::string methodName, std::string methodSignature) {
	jmethodID result = getJNIEnv()->GetStaticMethodID(classID, methodName.c_str(), methodSignature.c_str());

	if(!result) {
		gLoge("gAndroidUtil") << "couldn't find static method '"
							  << methodName << "' with signature '"
							  << methodSignature << "' in class '"
							  << getJavaClassName(classID) << "'";
		return nullptr;
	}

	return result;
}

std::string gAndroidUtil::getJavaClassName(jclass classID) {
	return "[UNKNOWN]"; //TODO
}

jclass gAndroidUtil::getJavaClassID(std::string className) {
	jclass result = getJNIEnv()->FindClass(className.c_str());

	if(!result) {
		gLoge("gAndroidUtil::getJavaClassID") << "couldn't find class '"
											  << className << "'";
		return nullptr;
	}

	return result;
}

jfieldID gAndroidUtil::getJavaStaticFieldID(jclass classID, std::string fieldName, std::string fieldType) {
	jfieldID result = getJNIEnv()->GetStaticFieldID(classID, fieldName.c_str(), fieldType.c_str());
	if(!result){
		gLoge("gAndroidUtil::getJavaStaticFieldID") << "couldn't find static field '" <<
				fieldName << "' of type '" <<
				fieldType << "' in class '" <<
				gAndroidUtil::getJavaClassName(classID) << "'";
		return nullptr;
	}

	return result;
}

jobject gAndroidUtil::getJavaStaticObjectField(jclass classID, std::string fieldName, std::string fieldType) {
	jfieldID fieldID = gAndroidUtil::getJavaStaticFieldID(classID, fieldName, fieldType);
	if (!fieldID)
		return nullptr;

	return getJNIEnv()->GetStaticObjectField(classID, fieldID);
}

jobject gAndroidUtil::getJavaStaticObjectField(std::string className, std::string fieldName, std::string fieldType) {
	jclass classID = gAndroidUtil::getJavaClassID(className);
	if (!classID)
		return nullptr;

	jobject result = gAndroidUtil::getJavaStaticObjectField(classID, fieldName, fieldType);
	getJNIEnv()->DeleteLocalRef(classID);

	return result;
}

void gAndroidUtil::callJavaVoidMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, va_list args){
	jmethodID methodID = gAndroidUtil::getJavaMethodID(classID, methodName, methodSignature);
	if (!methodID)
		return;

	getJNIEnv()->CallVoidMethodV(object, methodID, args);
}

void gAndroidUtil::callJavaVoidMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, ...) {
	va_list args;
	va_start(args, methodSignature);
	gAndroidUtil::callJavaVoidMethod(object, classID, methodName, methodSignature, args);
	va_end(args);
}

void gAndroidUtil::callJavaVoidMethod(jobject object, std::string className, std::string methodName, std::string methodSignature, ...) {
	jclass classID = gAndroidUtil::getJavaClassID(className);
	if (!classID)
		return;

	va_list args;
	va_start(args, methodSignature);
	gAndroidUtil::callJavaVoidMethod(object, classID, methodName, methodSignature, args);
	va_end(args);
	getJNIEnv()->DeleteLocalRef(classID);
}

jobject gAndroidUtil::callJavaStaticObjectMethod(jclass classID, std::string methodName, std::string methodSignature, va_list args)
{
	jmethodID methodID = gAndroidUtil::getJavaStaticMethodID(classID, methodName, methodSignature);

	if (!methodID)
		return nullptr;

	return getJNIEnv()->CallStaticObjectMethodV(classID, methodID, args);
}

jobject gAndroidUtil::callJavaStaticObjectMethod(jclass classID, std::string methodName, std::string methodSignature, ...) {
	va_list args;
	va_start(args, methodSignature);
	jobject result = gAndroidUtil::callJavaStaticObjectMethod(classID, methodName, methodSignature, args);
	va_end(args);

	return result;
}

jobject gAndroidUtil::callJavaStaticObjectMethod(std::string className, std::string methodName, std::string methodSignature, ...) {
	jclass classID = gAndroidUtil::getJavaClassID(className);
	if (!classID)
		return nullptr;
	va_list args;
	va_start(args, methodSignature);
	jobject result = gAndroidUtil::callJavaStaticObjectMethod(classID, methodName, methodSignature, args);
	va_end(args);
	getJNIEnv()->DeleteLocalRef(classID);

	return result;
}

jobject gAndroidUtil::callJavaObjectMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, va_list args) {
	jmethodID methodID = gAndroidUtil::getJavaMethodID(classID, methodName, methodSignature);
	if (!methodID)
		return nullptr;

	return getJNIEnv()->CallObjectMethodV(object, methodID, args);
}

jobject gAndroidUtil::callJavaObjectMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, ...) {
	va_list args;
	va_start(args, methodSignature);
	jobject result = gAndroidUtil::callJavaObjectMethod(object, classID, methodName, methodSignature, args);
	va_end(args);

	return result;
}

jobject gAndroidUtil::callJavaObjectMethod(jobject object, std::string className, std::string methodName, std::string methodSignature, ...) {
	jclass classID = gAndroidUtil::getJavaClassID(className);
	if (!classID)
		return nullptr;

	va_list args;
	va_start(args, methodSignature);
	jobject result = gAndroidUtil::callJavaObjectMethod(object, classID, methodName, methodSignature, args);
	va_end(args);
	getJNIEnv()->DeleteLocalRef(classID);

	return result;
}


void gAndroidUtil::callJavaStaticVoidMethod(jclass classID, std::string methodName, std::string methodSignature, va_list args) {
	jmethodID methodID = gAndroidUtil::getJavaStaticMethodID(classID, methodName, methodSignature);
	if (!methodID)
		return;

	getJNIEnv()->CallStaticVoidMethodV(classID, methodID, args);
}

void gAndroidUtil::callJavaStaticVoidMethod(jclass classID, std::string methodName, std::string methodSignature, ...) {
	va_list args;
	va_start(args, methodSignature);
	gAndroidUtil::callJavaStaticVoidMethod(classID, methodName, methodSignature, args);
	va_end(args);
}

void gAndroidUtil::callJavaStaticVoidMethod(std::string className, std::string methodName, std::string methodSignature, ...) {
	jclass classID = gAndroidUtil::getJavaClassID(className);
	if (!classID)
		return;

	va_list args;
	va_start(args, methodSignature);
	gAndroidUtil::callJavaStaticVoidMethod(classID, methodName, methodSignature, args);
	va_end(args);
	getJNIEnv()->DeleteLocalRef(classID);
}

float gAndroidUtil::callJavaFloatMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, va_list args) {
	jmethodID methodID = gAndroidUtil::getJavaMethodID(classID, methodName, methodSignature);
	if (!methodID) {
		gLoge("gAndroidUtil") << "Couldn't find " << methodName << " for float call";
		return 0;
	}

	return getJNIEnv()->CallFloatMethodV(object, methodID, args);
}

float gAndroidUtil::callJavaFloatMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, ...) {
	va_list args;
	va_start(args, methodSignature);
	auto result = gAndroidUtil::callJavaFloatMethod(object, classID, methodName, methodSignature, args);
	va_end(args);

	return result;
}

float gAndroidUtil::callJavaFloatMethod(jobject object, std::string className, std::string methodName, std::string methodSignature, ...) {
	jclass classID = gAndroidUtil::getJavaClassID(className);
	if (!classID) {
		gLoge() << "Couldn't find " << className << " for float call";
		return 0;
	}

	va_list args;
	va_start(args, methodSignature);
	auto result = gAndroidUtil::callJavaFloatMethod(object, classID, methodName, methodSignature, args);
	va_end(args);
	getJNIEnv()->DeleteLocalRef(classID);

	return result;
}

int gAndroidUtil::callJavaIntMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, va_list args){
	jmethodID methodID = gAndroidUtil::getJavaMethodID(classID, methodName, methodSignature);

	if (!methodID) {
		gLoge() << "Couldn't find " << methodName << " for int call";
		return 0;
	}

	return getJNIEnv()->CallIntMethodV(object, methodID, args);
}

int gAndroidUtil::callJavaIntMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, ...) {
	va_list args;
	va_start(args, methodSignature);
	auto result = gAndroidUtil::callJavaIntMethod(object, classID, methodName, methodSignature, args);
	va_end(args);

	return result;
}

int gAndroidUtil::callJavaIntMethod(jobject object, std::string className, std::string methodName, std::string methodSignature, ...) {
	jclass classID = gAndroidUtil::getJavaClassID(className);

	if (!classID) {
		gLoge() << "Couldn't find " << className << " for int call";
		return 0;
	}

	va_list args;
	va_start(args, methodSignature);
	auto result = gAndroidUtil::callJavaIntMethod(object, classID, methodName, methodSignature, args);
	va_end(args);
	getJNIEnv()->DeleteLocalRef(classID);

	return result;
}

int64_t gAndroidUtil::callJavaLongMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, va_list args){
	jmethodID methodID = gAndroidUtil::getJavaMethodID(classID, methodName, methodSignature);
	if (!methodID) {
		gLoge() << "Couldn't find " << methodName << " for int64_t call";
		return 0;
	}

	return getJNIEnv()->CallLongMethodV(object, methodID, args);
}

int64_t gAndroidUtil::callJavaLongMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, ...) {
	va_list args;
	va_start(args, methodSignature);
	auto result = gAndroidUtil::callJavaLongMethod(object, classID, methodName, methodSignature, args);
	va_end(args);

	return result;
}

int64_t gAndroidUtil::callJavaLongMethod(jobject object, std::string className, std::string methodName, std::string methodSignature, ...) {
	jclass classID = gAndroidUtil::getJavaClassID(className);

	if (!classID) {
		gLoge() << "Couldn't find " << className << " for int64_t call";
		return 0;
	}

	va_list args;
	va_start(args, methodSignature);
	auto result = gAndroidUtil::callJavaLongMethod(object, classID, methodName, methodSignature, args);
	va_end(args);
	getJNIEnv()->DeleteLocalRef(classID);

	return result;
}

bool gAndroidUtil::callJavaBoolMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, va_list args) {
	jmethodID methodID = gAndroidUtil::getJavaMethodID(classID, methodName, methodSignature);

	if (!methodID) {
		gLoge() << "Couldn't find " << methodName << " for bool call";
		return false;
	}

	return getJNIEnv()->CallBooleanMethodV(object, methodID, args);
}

bool gAndroidUtil::callJavaBoolMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, ...){
	va_list args;
	va_start(args, methodSignature);
	auto result = gAndroidUtil::callJavaBoolMethod(object, classID, methodName, methodSignature, args);
	va_end(args);

	return result;
}

bool gAndroidUtil::callJavaBoolMethod(jobject object, std::string className, std::string methodName, std::string methodSignature, ...){
	jclass classID = gAndroidUtil::getJavaClassID(className);
	if (!classID) {
		gLoge("gAndroidUtil") << "Couldn't find " << className << " for bool call";
		return false;
	}

	va_list args;
	va_start(args, methodSignature);
	auto result = gAndroidUtil::callJavaBoolMethod(object, classID, methodName, methodSignature, args);
	va_end(args);
	getJNIEnv()->DeleteLocalRef(classID);

	return result;
}

void gAndroidUtil::convertStringToJString(const std::string& str, jstring &jstr) {
	jstr = getJNIEnv()->NewStringUTF(str.c_str());
}

void gAndroidUtil::convertJStringToString(JNIEnv * env, jstring jstr, std::string &str) {
	jboolean isCopy;
	const char *arrayChar = env->GetStringUTFChars(jstr, &isCopy);
	if(isCopy == JNI_FALSE)
		return;
	str = arrayChar;
}

JavaVM* gAndroidUtil::getJavaVM() {
	return javavm;
}

JNIEnv* gAndroidUtil::getJNIEnv() {
	JNIEnv *env;
	JavaVM * vm = getJavaVM();
	if(!vm) {
		gLoge("gAndroidUtil") << "couldn't get java virtual machine";
		return nullptr;
	}

	int getEnvStat = vm->GetEnv((void**) &env, JNI_VERSION_1_4);

	if (getEnvStat == JNI_EDETACHED) {
		if (vm->AttachCurrentThread(&env, nullptr) != 0) {
			gLoge("gAndroidUtil") << "couldn't get environment using GetEnv()";
			return nullptr;
		}
	} else if (getEnvStat != JNI_OK) {
		gLoge("gAndroidUtil") << "couldn't get environment using GetEnv()";
		return nullptr;
	}

	return env;
}

jclass gAndroidUtil::getJavaGlistAndroid() {
	return getJNIEnv()->FindClass("dev/glist/android/lib/GlistNative");
}

void gAndroidUtil::setDeviceOrientation(DeviceOrientation orientation) {
	jclass glistandroid = getJavaGlistAndroid();

	jmethodID method = getJNIEnv()->GetStaticMethodID(glistandroid,"setDeviceOrientation","(I)V");
	getJNIEnv()->CallStaticVoidMethod(glistandroid,method, orientation);
}



extern "C" {

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	JNIEnv *env;
	javavm = vm;
	gLogd("gAndroidUtil") << "JNI_OnLoad called";
	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		gLogd("gAndroidUtil") << "failed to get environment using GetEnv()";
		return -1;
	}

	return JNI_VERSION_1_4;
}

JNIEXPORT void JNICALL Java_dev_glist_android_lib_GlistNative_setAssetManager(JNIEnv* env, jclass clazz, jobject assets) {

	AAssetManager* man = AAssetManager_fromJava(env, assets);
	gAndroidUtil::assets = man;
}

}
