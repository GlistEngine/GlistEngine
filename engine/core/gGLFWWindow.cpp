/*
 * gGLFWWindow.cpp
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

#include "gGLFWWindow.h"
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include "stb/stb_image.h"


gGLFWWindow::gGLFWWindow() {
#if defined(WIN32) || defined(LINUX) || defined(APPLE)
	window = nullptr;
#endif
}

gGLFWWindow::~gGLFWWindow() {
}


void gGLFWWindow::initialize(int width, int height, int windowMode) {
	gBaseWindow::initialize(width, height, windowMode);
#if defined(WIN32) || defined(LINUX) || defined(APPLE)
	// Create glfw
	glfwInit();


	// Configure glfw
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_IPHONE_SIMULATOR
        // define something for simulator
    #elif TARGET_OS_IPHONE
        // define something for iphone
    #else
        #define TARGET_OS_OSX 1
        // define something for OSX
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //case_mac
    #endif
#endif

	// All hints available at https://www.glfw.org/docs/latest/window.html#window_hints


	// Create window
	int currentrefreshrate = 60;
    if (windowMode == gBaseWindow::WINDOWMODE_GAME) {
    	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    	width = mode->width;
    	height = mode->height;
    	currentrefreshrate = mode->refreshRate;
    } else if (windowMode == gBaseWindow::WINDOWMODE_FULLSCREEN) {
    	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    }

    window = glfwCreateWindow(width, height, title.c_str(),
			(windowMode==gBaseWindow::WINDOWMODE_GAME?glfwGetPrimaryMonitor():NULL), NULL);


	if (window == NULL) {
	    std::cout << "Failed to create GLFW window" << std::endl;
	    glfwTerminate();
	    return;
	} else {
	    if (windowMode == gBaseWindow::WINDOWMODE_GAME) {
	    	GLFWmonitor* monitor = glfwGetWindowMonitor(window);
	    	glfwSetWindowMonitor(window, monitor, 0, 0, width, height, currentrefreshrate);
	    }
	}
	glfwGetWindowSize(window, &width, &height);

#ifndef LINUX
	GLFWimage images[1];
	std::string iconpath = gGetImagesDir() + "gameicon/icon.png";
	images[0].pixels = stbi_load(iconpath.c_str(), &images[0].width, &images[0].height, 0, 4); //rgba channels
	glfwSetWindowIcon(window, 1, images);
	stbi_image_free(images[0].pixels);
#endif

	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	glewInit();

#if defined(WIN32) || defined(LINUX) || defined(APPLE)
//	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
//	    std::cout << "Failed to initialize GLAD" << std::endl;
//	    return -1;
//	}
#endif

	glfwSetWindowUserPointer(window, this);


	// Window specs to OpenGL
	glViewport(0, 0, width, height);


	// Notify OpenGL if the window size changed
	glfwSetFramebufferSizeCallback(window, gGLFWWindow::framebuffer_size_callback);
	glfwSetKeyCallback(window, gGLFWWindow::key_callback);
	glfwSetCursorPosCallback(window, gGLFWWindow::mouse_pos_callback);
	glfwSetMouseButtonCallback(window, gGLFWWindow::mouse_button_callback);
	glfwSetCursorEnterCallback(window, gGLFWWindow::mouse_enter_callback);
#endif
}


bool gGLFWWindow::getShouldClose() {
#if defined(WIN32) || defined(LINUX) || defined(APPLE)
	return glfwWindowShouldClose(window);
#endif
	return 0;
}

void gGLFWWindow::update() {
#if defined(WIN32) || defined(LINUX) || defined(APPLE)
	// End window drawing
    glfwSwapBuffers(window);
    glfwPollEvents();
#endif
//    std::cout << "width:" << width << ", height:" << height << std::endl;
}

void gGLFWWindow::close() {
#if defined(WIN32) || defined(LINUX) || defined(APPLE)
	// Deallocate glfw resources
	glfwTerminate();
#endif
}

#if defined(WIN32) || defined(LINUX) || defined(APPLE)
void gGLFWWindow::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    (static_cast<gGLFWWindow *>(glfwGetWindowUserPointer(window)))->setSize(width, height);
}


void gGLFWWindow::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action != GLFW_RELEASE && action != GLFW_PRESS) return;
    (static_cast<gGLFWWindow *>(glfwGetWindowUserPointer(window)))->onKeyEvent(key, action);
}

void gGLFWWindow::mouse_pos_callback(GLFWwindow* window, double xpos, double ypos) {
	(static_cast<gGLFWWindow *>(glfwGetWindowUserPointer(window)))->onMouseMoveEvent(xpos, ypos);
}

void gGLFWWindow::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	double xpos, ypos;
   glfwGetCursorPos(window, &xpos, &ypos);
   (static_cast<gGLFWWindow *>(glfwGetWindowUserPointer(window)))->onMouseButtonEvent(button, action, xpos, ypos);
}

void gGLFWWindow::mouse_enter_callback(GLFWwindow* window, int entered) {
	(static_cast<gGLFWWindow *>(glfwGetWindowUserPointer(window)))->onMouseEnterEvent(entered);
}
#endif

