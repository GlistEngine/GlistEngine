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
#include <gWindowEvents.h>

void androidMain();

class gAndroidUtil {
public:
	static AAssetManager* assets;

	static AAsset* loadAsset(const std::string& path, int mode);
	static void closeAsset(AAsset* asset);

	static JavaVM* getJavaVM();
	static JNIEnv* getJNIEnv();

	static jclass getJavaGlistAndroid();

	static void setDeviceOrientation(DeviceOrientation orientation);

	jmethodID getJavaMethodID(jclass classID, std::string methodName, std::string methodSignature);
	jmethodID getJavaStaticMethodID(jclass classID, std::string methodName, std::string methodSignature);
	std::string getJavaClassName(jclass classID);
	jclass getJavaClassID(std::string className);
	jfieldID getJavaStaticFieldID(jclass classID, std::string fieldName, std::string fieldType);
	void convertStringToJString(const std::string& str, jstring &jstr);
	void convertJStringToString(JNIEnv* env, jstring jstr, std::string &str);

	jobject getJavaStaticObjectField(jclass classID, std::string fieldName, std::string fieldType);
	jobject getJavaStaticObjectField(std::string className, std::string fieldName, std::string fieldType);

	void callJavaVoidMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, va_list args);
	void callJavaVoidMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, ...);
	void callJavaVoidMethod(jobject object, std::string className, std::string methodName, std::string methodSignature, ...);

	jobject callJavaStaticObjectMethod(jclass classID, std::string methodName, std::string methodSignature, va_list args);
	jobject callJavaStaticObjectMethod(jclass classID, std::string methodName, std::string methodSignature, ...);
	jobject callJavaStaticObjectMethod(std::string className, std::string methodName, std::string methodSignature, ...);

	jobject callJavaObjectMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, va_list args);
	jobject callJavaObjectMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, ...);
	jobject callJavaObjectMethod(jobject object, std::string className, std::string methodName, std::string methodSignature, ...);

	void callJavaStaticVoidMethod(jclass classID, std::string methodName, std::string methodSignature, va_list args);
	void callJavaStaticVoidMethod(jclass classID, std::string methodName, std::string methodSignature, ...);
	void callJavaStaticVoidMethod(std::string className, std::string methodName, std::string methodSignature, ...);

	float callJavaFloatMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, va_list args);
	float callJavaFloatMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, ...);
	float callJavaFloatMethod(jobject object, std::string className, std::string methodName, std::string methodSignature, ...);

	int callJavaIntMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, va_list args);
	int callJavaIntMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, ...);
	int callJavaIntMethod(jobject object, std::string className, std::string methodName, std::string methodSignature, ...);

	int64_t callJavaLongMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, va_list args);
	int64_t callJavaLongMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, ...);
	int64_t callJavaLongMethod(jobject object, std::string className, std::string methodName, std::string methodSignature, ...);

	bool callJavaBoolMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, va_list args);
	bool callJavaBoolMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, ...);
	bool callJavaBoolMethod(jobject object, std::string className, std::string methodName, std::string methodSignature, ...);

};


#endif /* ANDROID */

#endif //GANDROIDUTIL_H
