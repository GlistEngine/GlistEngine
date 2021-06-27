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


class gGLFWWindow : public gBaseWindow {
public:
	gGLFWWindow();
	~gGLFWWindow();

	void initialize(int width, int height, int windowMode) override;
	bool getShouldClose();
	void update();
	void close();

private:
#if defined(WIN32) || defined(LINUX) || defined(APPLE)
	GLFWwindow* window;
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void character_callback(GLFWwindow* window, unsigned int keycode);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouse_pos_callback(GLFWwindow* window, double xpos, double ypos);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void mouse_enter_callback(GLFWwindow* window, int entered);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
#endif
};

#endif /* ENGINE_CORE_GGLFWWINDOW_H_ */
