/*
 * gGLFWWindow.h
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

#ifndef ENGINE_CORE_GGLFWWINDOW_H_
#define ENGINE_CORE_GGLFWWINDOW_H_

#include "gBaseWindow.h"
// #include <glad/glad.h> //case_win
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "gCamera.h"

/**
 * gGLFWWindow, contain functions for game's window.
 *
 * Uses parent as gBaseWindow.
 */

class gGLFWWindow : public gBaseWindow {
public:
	static constexpr gCursorMode CURSORMODE_NORMAL = gCursorMode::CURSORMODE_NORMAL;
	static constexpr gCursorMode CURSORMODE_HIDDEN = gCursorMode::CURSORMODE_HIDDEN;
	static constexpr gCursorMode CURSORMODE_DISABLED = gCursorMode::CURSORMODE_DISABLED;
	static constexpr gCursorMode CURSORMODE_RELATIVE = gCursorMode::CURSORMODE_RELATIVE;

	gGLFWWindow();
	~gGLFWWindow() override;

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
	void setCursorMode(gCursorMode cursorMode) override;
	void setCursorPos(int x, int y) override;

	void setClipboardString(std::string text) override;
	std::string getClipboardString() override;

	void setWindowSize(int width, int height) override;
	void setWindowResizable(bool isResizable) override;
	void setWindowSizeLimits(int minWidth, int minHeight, int maxWidth, int maxHeight) override;

	void setIcon(std::string pngFullpath) override;
	void setIcon(unsigned char* imageData, int w, int h) override;

	void setTitle(const std::string& windowTitle) override;

	bool isJoystickPresent(int joystickId) override;
	bool isGamepadButtonPressed(int joystickId, int buttonId) override;
	const float* getJoystickAxes(int joystickId, int* axisCountPtr) override;

	float getScaleX() const {
		return scalex;
	}
	float getScaleY() const {
		return scaley;
	}

	void setScale(float x, float y);

  private:
	GLFWwindow* window;
	GLFWcursor** cursor;
	float scalex, scaley;

};

#endif /* ENGINE_CORE_GGLFWWINDOW_H_ */
