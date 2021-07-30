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
#if defined(WIN32) || defined(LINUX) || defined(APPLE)
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#endif
#include "gCamera.h"

/**
 * gGLFWWindow, contain functions for game's window.
 *
 * Uses parent as gBaseWindow.
 */

class gGLFWWindow : public gBaseWindow {
public:
	gGLFWWindow();
	~gGLFWWindow();

	/**
	 * Assigns the entered values ​​before the game window opens.
	 *
	 * @param width Sets what the width of the window should be.
	 * @param height Sets what the height of the window should be.
	 * @param windowMode Determines which mode the window is in. Example: Window Mode,Game Mode, Fullscreen Mode.
	 */
	void initialize(int width, int height, int windowMode) override;

	/**
	 * Returns if the window should be closed or not.
	 *
	 * @return bool varaible and says that windows should be closed or not.
	 */
	bool getShouldClose();

	/**
	 * Performs the said operations at the specified frame rate.
	 */
	void update();

	/**
	 * Destroys all remaining windows and cursors, restores any modified gamma ramps and frees any other allocated resources.
	 */
	void close();

private:
#if defined(WIN32) || defined(LINUX) || defined(APPLE)
	GLFWwindow* window;

	/**
	 * Invoking by GLFW if the window size changed.
	 */
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void character_callback(GLFWwindow* window, unsigned int keycode);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

	/**
	 * Invoking by GLFW if mouse position changed.
	 */
	static void mouse_pos_callback(GLFWwindow* window, double xpos, double ypos);

	/**
	 * Invoking by GLFW if mouse button pressed with mouse position.
	 */
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

	/**
	 * Invoking by GLFW if mouse entered window.
	 */
	static void mouse_enter_callback(GLFWwindow* window, int entered);

	/**
	 * Invoking by GLFW if scroll changed.
	 */
	static void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
#endif
};

#endif /* ENGINE_CORE_GGLFWWINDOW_H_ */
