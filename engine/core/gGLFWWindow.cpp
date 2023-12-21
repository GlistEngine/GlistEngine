/*
 * gGLFWWindow.cpp
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

#include "gGLFWWindow.h"
#include "gAppManager.h"

const int gGLFWWindow::CURSORMODE_NORMAL = GLFW_CURSOR_NORMAL;
const int gGLFWWindow::CURSORMODE_HIDDEN = GLFW_CURSOR_HIDDEN;
const int gGLFWWindow::CURSORMODE_DISABLED = GLFW_CURSOR_DISABLED;

GLFWwindow* gGLFWWindow::currentwindow = nullptr;


gGLFWWindow::gGLFWWindow() {
	window = nullptr;
	cursor = new GLFWcursor*[6];
	scalex = 1.0f;
	scaley = 1.0f;
}

gGLFWWindow::~gGLFWWindow() {
}


void gGLFWWindow::initialize(int width, int height, int windowMode, bool isResizable) {
	gBaseWindow::initialize(width, height, windowMode, isResizable);
	// Create glfw
	glfwInit();


	// Configure glfw
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if TARGET_OS_OSX
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //case_mac
#endif

	// All hints available at https://www.glfw.org/docs/latest/window.html#window_hints


	// Create window
	int currentrefreshrate = 60;
    if(windowMode == G_WINDOWMODE_GAME) {
    	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    	width = mode->width;
    	height = mode->height;
    	currentrefreshrate = mode->refreshRate;
    } else if(windowMode == G_WINDOWMODE_FULLSCREEN || windowMode == G_WINDOWMODE_FULLSCREENGUIAPP) {
    	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    	glfwWindowHint(GLFW_RESIZABLE, isResizable);
    } else {
    	glfwWindowHint(GLFW_RESIZABLE, isResizable);
    }

    window = glfwCreateWindow(width, height, title.c_str(),
			(windowMode == G_WINDOWMODE_GAME?glfwGetPrimaryMonitor():NULL), NULL);

	if(window == NULL) {
	    std::cout << "Failed to create GLFW window" << std::endl;
	    glfwTerminate();
	    return;
	}
    currentwindow = window;

	if(windowMode == G_WINDOWMODE_GAME) {
	   	GLFWmonitor* monitor = glfwGetWindowMonitor(window);
	   	glfwSetWindowMonitor(window, monitor, 0, 0, width, height, currentrefreshrate);
	   	glViewport(0, 0, width, height);
	}

	glfwGetFramebufferSize(window, &width, &height);
	this->width = width;
	this->height = height;

	// Fix mouse movement for HiDPI (scaled) displays.
	int windowWidth;
	int windowHeight;
	glfwGetWindowSize(window, &windowWidth, &windowHeight);
	// Currently, this is not updated from anywhere, we might need to update this value inside framebuffer_size_callback in some rare edge case.
	this->scalex = (float) width / (float) windowWidth;
	this->scaley = (float) height / (float) windowHeight;

	GLFWimage images[1];
	std::string iconpath = gGetImagesDir() + "gameicon/icon.png";
	images[0].pixels = stbi_load(iconpath.c_str(), &images[0].width, &images[0].height, 0, 4); //rgba channels
	glfwSetWindowIcon(window, 1, images);
	stbi_image_free(images[0].pixels);

	cursor[0] = glfwCreateStandardCursor(0x00036001);
	cursor[1] = glfwCreateStandardCursor(0x00036002);
	cursor[2] = glfwCreateStandardCursor(0x00036003);
	cursor[3] = glfwCreateStandardCursor(0x00036004);
	cursor[4] = glfwCreateStandardCursor(0x00036005);
	cursor[5] = glfwCreateStandardCursor(0x00036006);
	glfwSetCursor(window, cursor[0]);

	glfwMakeContextCurrent(window);
    glfwSwapInterval(vsync ? 1 : 0);
	glewExperimental = GL_TRUE;
	glewInit();

//	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
//	    std::cout << "Failed to initialize GLAD" << std::endl;
//	    return -1;
//	}

	glfwSetWindowUserPointer(window, this);


	// Window specs to OpenGL
	glViewport(0, 0, width, height);


	// Notify OpenGL if the window size changed
	glfwSetFramebufferSizeCallback(window, gGLFWWindow::framebuffer_size_callback);
	glfwSetCharCallback(window, gGLFWWindow::character_callback);
	glfwSetKeyCallback(window, gGLFWWindow::key_callback);
	glfwSetCursorPosCallback(window, gGLFWWindow::mouse_pos_callback);
	glfwSetMouseButtonCallback(window, gGLFWWindow::mouse_button_callback);
	glfwSetCursorEnterCallback(window, gGLFWWindow::mouse_enter_callback);
	glfwSetScrollCallback(window, gGLFWWindow::mouse_scroll_callback);
	glfwSetWindowFocusCallback(window, gGLFWWindow::window_focus_callback);
	glfwSetJoystickCallback(gGLFWWindow::joystick_callback);
}

bool gGLFWWindow::getShouldClose() {
	return glfwWindowShouldClose(window);
}

void gGLFWWindow::update() {
	// End window drawing
	G_CHECK_GL(glfwSwapBuffers(window));
	G_CHECK_GL(glfwPollEvents());
//    std::cout << "width:" << width << ", height:" << height << std::endl;
}

void gGLFWWindow::close() {
	// Deallocate glfw resources
	glfwTerminate();
}

void gGLFWWindow::setVsync(bool vsync) {
	gBaseWindow::setVsync(vsync);
	glfwSwapInterval(vsync);
}

void gGLFWWindow::setCursor(int cursorNo) {
	glfwSetCursor(window, cursor[cursorNo]);
}

void gGLFWWindow::setCursorMode(int cursorMode) {
	glfwSetInputMode(window, GLFW_CURSOR, cursorMode);
}

void gGLFWWindow::setCursorPos(int x, int y) {
	glfwSetCursorPos(window, x / scalex, y / scaley);
}

void gGLFWWindow::setClipboardString(std::string text) {
	glfwSetClipboardString(window, text.c_str());
}

std::string gGLFWWindow::getClipboardString() {
	return glfwGetClipboardString(window);
}

void gGLFWWindow::setWindowSize(int width, int height) {
	if(window != nullptr) {
		glfwSetWindowSize(window, width, height);
		framebuffer_size_callback(window, width, height);
	}
}

void gGLFWWindow::setWindowResizable(bool isResizable) {
	if(window != nullptr) {
		glfwSetWindowAttrib(window, GLFW_RESIZABLE, isResizable);
	}
}

void gGLFWWindow::setWindowSizeLimits(int minWidth, int minHeight, int maxWidth, int maxHeight) {
	if(window != nullptr) {
		if(minWidth == 0) minWidth = GLFW_DONT_CARE;
		if(minHeight == 0) minHeight = GLFW_DONT_CARE;
		if(maxWidth == 0) maxWidth = GLFW_DONT_CARE;
		if(maxHeight == 0) maxHeight = GLFW_DONT_CARE;
		glfwSetWindowSizeLimits(window, minWidth, minHeight, maxWidth, maxHeight);
	}
}

bool gGLFWWindow::isJoystickPresent(int joystickId) {
	return glfwJoystickPresent(joystickId);
}

bool gGLFWWindow::isGamepadButtonPressed(int joystickId, int buttonId) {
	GLFWgamepadstate gpstate;
	glfwGetGamepadState(joystickId, &gpstate);
	return gpstate.buttons[buttonId];
}

const float* gGLFWWindow::getJoystickAxes(int joystickId, int* axisCountPtr) {
	return glfwGetJoystickAxes(joystickId, axisCountPtr);
}

void gGLFWWindow::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
	auto handle = static_cast<gGLFWWindow*>(glfwGetWindowUserPointer(window));
    handle->setSize(width, height);
}

void gGLFWWindow::character_callback(GLFWwindow* window, unsigned int keycode) {
	auto handle = static_cast<gGLFWWindow*>(glfwGetWindowUserPointer(window));
	gCharTypedEvent event{keycode};
	handle->callEvent(event);
}

void gGLFWWindow::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action != GLFW_RELEASE && action != GLFW_PRESS) return;
	auto handle = static_cast<gGLFWWindow*>(glfwGetWindowUserPointer(window));
	switch(action) {
	case GLFW_RELEASE:{
		gKeyReleasedEvent event{key};
		handle->callEvent(event);
		break;
	}
	case GLFW_PRESS:{
		gKeyPressedEvent event{key};
		handle->callEvent(event);
		break;
	}
	}
}

void gGLFWWindow::window_focus_callback(GLFWwindow* window, int focused) {
	auto handle = static_cast<gGLFWWindow*>(glfwGetWindowUserPointer(window));
	if(focused) {
		gWindowFocusEvent event{};
		handle->callEvent(event);
	} else {
		gWindowLoseFocusEvent event{};
		handle->callEvent(event);
	}
}

void gGLFWWindow::joystick_callback(int jid, int action) {
	auto handle = static_cast<gGLFWWindow*>(glfwGetWindowUserPointer(currentwindow));
	switch(action) {
	case GLFW_CONNECTED: {
		gJoystickConnectEvent event{jid, glfwJoystickIsGamepad(jid) == GLFW_TRUE};
		handle->callEvent(event);
		break;
	}
	case GLFW_DISCONNECTED: {
		gJoystickDisconnectEvent event{jid};
		handle->callEvent(event);
		break;
	}
	}
}

void gGLFWWindow::mouse_pos_callback(GLFWwindow* window, double xpos, double ypos) {
	auto handle = static_cast<gGLFWWindow*>(glfwGetWindowUserPointer(window));
	gMouseMovedEvent event{static_cast<int>(xpos * handle->scalex), static_cast<int>(ypos * handle->scaley)};
	handle->callEvent(event);
}

void gGLFWWindow::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	auto handle = static_cast<gGLFWWindow*>(glfwGetWindowUserPointer(window));
	switch(action) {
	case GLFW_RELEASE:{
		gMouseButtonReleasedEvent event{button, static_cast<int>(xpos * handle->scalex), static_cast<int>(ypos * handle->scaley)};
		handle->callEvent(event);
		break;
	}
	case GLFW_PRESS:{
		gMouseButtonPressedEvent event{button, static_cast<int>(xpos * handle->scalex), static_cast<int>(ypos * handle->scaley)};
		handle->callEvent(event);
		break;
	}
	}
}

void gGLFWWindow::mouse_enter_callback(GLFWwindow* window, int entered) {
	auto handle = static_cast<gGLFWWindow*>(glfwGetWindowUserPointer(window));
	if(entered) {
		gWindowMouseEnterEvent event{};
		handle->callEvent(event);
	} else {
		gWindowMouseExitEvent event{};
		handle->callEvent(event);
	}
}

void gGLFWWindow::mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	auto handle = static_cast<gGLFWWindow*>(glfwGetWindowUserPointer(window));
	gMouseScrolledEvent event{static_cast<int>(xoffset), static_cast<int>(yoffset)};
	handle->callEvent(event);
}

