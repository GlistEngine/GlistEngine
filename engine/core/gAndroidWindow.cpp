/*
 * gAndroidWindow.cpp
 *
 *  Created on: June 24, 2023
 *      Author: Metehan Gezer
 */

#include "gAndroidWindow.h"
#include "gAppManager.h"

#ifdef ANDROID

static void onErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
	gLogi("gAndroidWindow") << "OpenGL ERROR: " << message;
}

ANativeWindow* gAndroidWindow::nativewindow;
gAndroidWindow* window;

gAndroidWindow::gAndroidWindow() {
    window = this;
    shouldclose = false;
}

gAndroidWindow::~gAndroidWindow() {
}

void gAndroidWindow::initialize(int uwidth, int uheight, int windowMode, bool isResizable) {
	gBaseWindow::initialize(uwidth, uheight, windowMode, isResizable);

	while(!nativewindow){ }

	const EGLint attribs[] = {
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT, // request OpenGL ES 3.0
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 16,
            EGL_NONE
	};
	EGLConfig config;
	EGLint numConfigs;
	EGLint format;


	if ((display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
		gLogi("gAndroidWindow") << "eglGetDisplay() returned error " << eglGetError();
        exit(-1);
		return;
	}
	if (!eglInitialize(display, 0, 0)) {
		gLogi("gAndroidWindow") << "eglInitialize() returned error " << eglGetError();
        exit(-1);
		return;
	}

	if (!eglChooseConfig(display, attribs, &config, 1, &numConfigs)) {
		gLogi("gAndroidWindow") << "eglChooseConfig() returned error " << eglGetError();
		close();
		return;
	}

	if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format)) {
		gLogi("gAndroidWindow") << "eglGetConfigAttrib() returned error " << eglGetError();
		close();
		return;
	}

	if (!(surface = eglCreateWindowSurface(display, config, nativewindow, 0))) {
		gLogi("gAndroidWindow") << "eglCreateWindowSurface() returned error " << eglGetError();
		close();
		return;
	}
    const EGLint attribList[] = {
			EGL_CONTEXT_CLIENT_VERSION, 3,
			EGL_NONE
	};

	if (!(context = eglCreateContext(display, config, EGL_NO_CONTEXT, attribList))) {
		gLogi("gAndroidWindow") << "eglCreateContext() returned error " << eglGetError();
		close();
		return;
	}

	if (!eglMakeCurrent(display, surface, surface, context)) {
		gLogi("gAndroidWindow") << "eglMakeCurrent() returned error " << eglGetError();
		close();
		return;
	}

	if (!eglQuerySurface(display, surface, EGL_WIDTH, &width) ||
		!eglQuerySurface(display, surface, EGL_HEIGHT, &height)) {
		gLogi("gAndroidWindow") << "eglQuerySurface() returned error " << eglGetError();
		close();
		return;
	}

	glViewport(0, 0, width, height);
	this->scalex = (float) width / (float) uwidth;
	this->scaley = (float) height / (float) uheight;
}


bool gAndroidWindow::getShouldClose() {
	return shouldclose;
}

void gAndroidWindow::update() {
	if (!eglSwapBuffers(display, surface)) {
		gLogi("gAndroidWindow") << "eglSwapBuffers() returned error " << eglGetError();
	}
	// End window drawing
//    std::cout << "width:" << width << ", height:" << height << std::endl;
}

void gAndroidWindow::close() {
	// todo
	eglTerminate(display);
}

void gAndroidWindow::enableVsync(bool vsync) {
	gBaseWindow::enableVsync(vsync);
}

void gAndroidWindow::setCursor(int cursorNo) {
}

void gAndroidWindow::setCursorMode(int cursorMode) {
}

void gAndroidWindow::setClipboardString(std::string text) {
}

std::string gAndroidWindow::getClipboardString() {
	return "ASD";
}

void gAndroidWindow::setWindowSize(int width, int height) {
}

void gAndroidWindow::setWindowResizable(bool isResizable) {
}

void gAndroidWindow::setWindowSizeLimits(int minWidth, int minHeight, int maxWidth, int maxHeight) {
}

bool gAndroidWindow::isRendering() {
    return isrendering;
}

bool gAndroidWindow::onTouchCallback(int pointerCount, int* fingerIds, int* x, int* y) {
	TouchInput inputs[pointerCount];
	for (int i = 0; i < pointerCount; ++i) {
		inputs[i] = {fingerIds[i], i, x[i], y[i]};
	}
	gTouchEvent event{pointerCount, inputs};
	callEvent(event);
	return false;
}

extern "C" {

JNIEXPORT void JNICALL Java_dev_glist_android_lib_GlistNative_setSurface(JNIEnv *env, jclass clazz, jobject surface) {
	if (surface != 0) {
		gAndroidWindow::nativewindow = ANativeWindow_fromSurface(env, surface);
	} else {
		ANativeWindow_release(gAndroidWindow::nativewindow);
	}
}

JNIEXPORT jboolean JNICALL Java_dev_glist_android_lib_GlistNative_onTouchEvent(JNIEnv *env, jclass clazz, jint pointerCount, jintArray fingerIds, jintArray x, jintArray y) {
    int* _fingerids = env->GetIntArrayElements(fingerIds, new jboolean(false));
    int* _x = env->GetIntArrayElements(x, new jboolean(false));
    int* _y = env->GetIntArrayElements(y, new jboolean(false));

	return window->onTouchCallback(pointerCount, _fingerids, _x, _y); // true if consumed
}

std::unique_ptr<std::thread> thread;
JNIEXPORT void JNICALL Java_dev_glist_android_lib_GlistNative_onCreate(JNIEnv *env, jclass clazz) {
    gLogi("GlistNative") << "onCreate";
    if(thread) {
        throw std::runtime_error("thread is already created!");
    }
    thread = std::make_unique<std::thread>([]() {
        androidMain();
    });
}

JNIEXPORT void JNICALL Java_dev_glist_android_lib_GlistNative_onDestroy(JNIEnv *env, jclass clazz) {
    gLogi("GlistNative") << "onDestroy";
    window->shouldclose = true;
    thread->join();
}

JNIEXPORT void JNICALL Java_dev_glist_android_lib_GlistNative_onStart(JNIEnv *env, jclass clazz) {
    gLogi("GlistNative") << "onStart";
}

JNIEXPORT void JNICALL Java_dev_glist_android_lib_GlistNative_onStop(JNIEnv *env, jclass clazz) {
    gLogi("GlistNative") << "onStop";
}

JNIEXPORT void JNICALL Java_dev_glist_android_lib_GlistNative_onPause(JNIEnv *env, jclass clazz) {
    gLogi("GlistNative") << "onPause";
    if (window) {
		// todo pause event
        window->isrendering = false;
    }
}

JNIEXPORT void JNICALL Java_dev_glist_android_lib_GlistNative_onResume(JNIEnv *env, jclass clazz) {
    gLogi("GlistNative") << "onResume";
    if (window) {
		// todo resume event
        window->isrendering = true;
    }
}

}

#endif /* ANDROID */