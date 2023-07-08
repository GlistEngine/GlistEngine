/*
 * gGLFWWindow.h
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

#ifndef GANDROIDWINDOW_H_
#define GANDROIDWINDOW_H_

#include "gBaseWindow.h"
#include "gCamera.h"
#include "gAppManager.h"
#include "gApp.h"
#include "gAndroidUtil.h"

#include <jni.h>
#include <string>
#include <iostream>
#include <android/log.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <fcntl.h>

#include <android/native_window.h> // requires ndk r5 or newer
#include <android/native_window_jni.h> // requires ndk r5 or newer
#include <EGL/egl.h> // requires ndk r5 or newer
#include <EGL/eglext.h> // requires ndk r5 or newer
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <thread>

class gAndroidWindow : public gBaseWindow {
public:
	static const int CURSORMODE_NORMAL, CURSORMODE_HIDDEN, CURSORMODE_DISABLED;

	gAndroidWindow();
	~gAndroidWindow();

	/**
	 * Assigns the entered values before the game window opens.
	 *
	 * @param width Sets what the width of the window should be.
	 * @param height Sets what the height of the window should be.
	 * @param windowMode Determines which mode the window is in. Example: Window Mode,Game Mode, Fullscreen Mode.
	 */
	void initialize(int width, int height, int windowMode, bool isResizable) override;

	/**
	 * Returns if the window should be closed or not.
	 *
	 * @return bool varaible and says that windows should be closed or not.
	 */
	bool getShouldClose() override;

	/**
	 * Performs the said operations at the specified frame rate.
	 */
	void update() override;

	/**
	 * Destroys all remaining windows and cursors, restores any modified gamma ramps and frees any other allocated resources.
	 */
	void close() override;

	void setVsync(bool vsync) override;

	void setCursor(int cursorNo) override;
	void setCursorMode(int cursorMode) override;

	void setClipboardString(std::string text) override;
	std::string getClipboardString() const override;

	void setWindowSize(int width, int height) override;
	void setWindowResizable(bool isResizable) override;
	void setWindowSizeLimits(int minWidth, int minHeight, int maxWidth, int maxHeight) override;

	bool onTouchCallback(int pointerCount, int* fingerIds, int* x, int* y);
public:
	static ANativeWindow* nativewindow;
	bool isclosed;

private:
	EGLDisplay display{};
	EGLSurface surface{};
	EGLContext context{};

};

#endif /* GANDROIDWINDOW_H_ */
