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

/**
 * A wrapper for JNI strings, automatically handles
 * deletion and creation of the reference.
 */
class JavaString {
public:
	JavaString(const std::string& string);
	~JavaString();

	constexpr jstring native() { return str; }
	constexpr operator jstring() { return str; }
	constexpr operator jobject() { return str; }
private:
	JNIEnv* env;
	jstring str;
};

class gAndroidUtil {
public:
	static AAssetManager* assets;

	static AAsset* loadAsset(const std::string& path, int mode);
	static void closeAsset(AAsset* asset);

	static JavaVM* getJavaVM();
	static JNIEnv* getJNIEnv();

	static jclass getJavaGlistAndroid();
	static jobject getJavaAndroidActivity();

	static void attachMainThread(jobject classloader);

	static void setDeviceOrientation(DeviceOrientation orientation);

	static jmethodID getJavaMethodID(jclass classID, std::string methodName, std::string methodSignature);
	static jmethodID getJavaStaticMethodID(jclass classID, std::string methodName, std::string methodSignature);
	static std::string getJavaClassName(jclass classID);
	static jclass getJavaClassID(std::string className);
	static jfieldID getJavaStaticFieldID(jclass classID, std::string fieldName, std::string fieldType);
	static void convertStringToJString(const std::string& str, jstring &jstr);
	static void convertJStringToString(JNIEnv* env, jstring jstr, std::string &str);

	static jobject getJavaStaticObjectField(jclass classID, std::string fieldName, std::string fieldType);
	static jobject getJavaStaticObjectField(std::string className, std::string fieldName, std::string fieldType);

	static void callJavaVoidMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, va_list args);
	static void callJavaVoidMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, ...);
	static void callJavaVoidMethod(jobject object, std::string className, std::string methodName, std::string methodSignature, ...);

	static jobject callJavaStaticObjectMethod(jclass classID, std::string methodName, std::string methodSignature, va_list args);
	static jobject callJavaStaticObjectMethod(jclass classID, std::string methodName, std::string methodSignature, ...);
	static jobject callJavaStaticObjectMethod(std::string className, std::string methodName, std::string methodSignature, ...);

	static jobject callJavaObjectMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, va_list args);
	static jobject callJavaObjectMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, ...);
	static jobject callJavaObjectMethod(jobject object, std::string className, std::string methodName, std::string methodSignature, ...);

	static void callJavaStaticVoidMethod(jclass classID, std::string methodName, std::string methodSignature, va_list args);
	static void callJavaStaticVoidMethod(jclass classID, std::string methodName, std::string methodSignature, ...);
	static void callJavaStaticVoidMethod(std::string className, std::string methodName, std::string methodSignature, ...);

	static float callJavaFloatMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, va_list args);
	static float callJavaFloatMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, ...);
	static float callJavaFloatMethod(jobject object, std::string className, std::string methodName, std::string methodSignature, ...);

	static int callJavaIntMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, va_list args);
	static int callJavaIntMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, ...);
	static int callJavaIntMethod(jobject object, std::string className, std::string methodName, std::string methodSignature, ...);

	static int64_t callJavaLongMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, va_list args);
	static int64_t callJavaLongMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, ...);
	static int64_t callJavaLongMethod(jobject object, std::string className, std::string methodName, std::string methodSignature, ...);

	static bool callJavaBoolMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, va_list args);
	static bool callJavaBoolMethod(jobject object, jclass classID, std::string methodName, std::string methodSignature, ...);
	static bool callJavaBoolMethod(jobject object, std::string className, std::string methodName, std::string methodSignature, ...);

};


#endif /* ANDROID */

#endif //GANDROIDUTIL_H
