/*
 * gGLFWWindow.cpp
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

// gVKContext.h decides whether Vulkan is available at all (GVK_VULKAN) and
// pulls in <vulkan/vulkan.h>. Both have to happen before GLFW is included below:
// glfwInitVulkanLoader is declared inside GLFW's #if defined(VK_VERSION_1_0)
// block, so without the Vulkan header first it would not exist.
#include "gVKContext.h"
#ifdef GVK_VULKAN
	#define GLFW_INCLUDE_VULKAN
#endif

#include "gGLFWWindow.h"
#include "gAppManager.h"
#include "gRenderObject.h"
#include "gTracy.h"
#ifdef WIN32
#include <ShellScalingApi.h>
#endif

// Static functions

static GLFWwindow* currentwindow = nullptr;

static void onFramebufferResize(GLFWwindow* window, int width, int height) {
	// A Vulkan window is created with GLFW_NO_API and has no current OpenGL
	// context. Calling glViewport from its resize callback is invalid and can crash
	// while the Vulkan backend is rebuilding the swapchain.
	if(glfwGetWindowAttrib(window, GLFW_CLIENT_API) != GLFW_NO_API) {
		glViewport(0, 0, width, height);
	}
	auto handle = static_cast<gGLFWWindow*>(glfwGetWindowUserPointer(window));
	if (handle) {
		handle->setSize(width, height);
	}
}

static void onCharInput(GLFWwindow* window, unsigned int keycode) {
	auto handle = static_cast<gGLFWWindow*>(glfwGetWindowUserPointer(window));
	if (handle) {
		gCharTypedEvent event{keycode};
		handle->callEvent(event);
	}
}

static void onKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action != GLFW_RELEASE && action != GLFW_PRESS) return;
	auto handle = static_cast<gGLFWWindow*>(glfwGetWindowUserPointer(window));
	if (!handle) return;

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

static void onWindowFocus(GLFWwindow* window, int focused) {
	auto handle = static_cast<gGLFWWindow*>(glfwGetWindowUserPointer(window));
	if (!handle) return;

	if(focused) {
		gWindowFocusEvent event{};
		handle->callEvent(event);
	} else {
		gWindowLoseFocusEvent event{};
		handle->callEvent(event);
	}
}

static void onScaleChange(GLFWwindow* window, float xscale, float yscale) {
	auto handle = static_cast<gGLFWWindow*>(glfwGetWindowUserPointer(window));
	if (handle) {
		handle->setScale(xscale, yscale);
	}
}

static void onJoystick(int jid, int action) {
	if (!currentwindow) {
		return;
	}
	auto handle = static_cast<gGLFWWindow*>(glfwGetWindowUserPointer(currentwindow));
	if (!handle) {
		return;
	}

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

static void onMouseMove(GLFWwindow* window, double xpos, double ypos) {
	auto handle = static_cast<gGLFWWindow*>(glfwGetWindowUserPointer(window));
	if (handle) {
		float x = xpos * handle->getScaleX();
		float y = ypos * handle->getScaleY();
		if (handle->getCursorMode() == CURSORMODE_RELATIVE) {
			// y is intentionally divided to width instead of height to get the same aspect ratio
			x = (handle->getWidth() / 2.0f - x) / handle->getWidth();
			y = (handle->getHeight() / 2.0f - y) / handle->getWidth();
		}
		gMouseMovedEvent event{
			x, y,
			handle->getCursorMode()
		};
		handle->callEvent(event);
		if (handle->getCursorMode() == CURSORMODE_RELATIVE) {
			handle->setCursorPos(handle->getWidth() / 2.0f,
				handle->getHeight() / 2.0f);
		}
	}
}

static void onMouseButton(GLFWwindow* window, int button, int action, int mods) {
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	auto handle = static_cast<gGLFWWindow*>(glfwGetWindowUserPointer(window));
	if (!handle) return;

	switch(action) {
	case GLFW_RELEASE:{
		gMouseButtonReleasedEvent event{button, static_cast<int>(xpos * handle->getScaleX()), static_cast<int>(ypos * handle->getScaleY())};
		handle->callEvent(event);
		break;
	}
	case GLFW_PRESS:{
		gMouseButtonPressedEvent event{button, static_cast<int>(xpos * handle->getScaleX()), static_cast<int>(ypos * handle->getScaleY())};
		handle->callEvent(event);
		break;
	}
	}
}

static void onMouseEnter(GLFWwindow* window, int entered) {
	auto handle = static_cast<gGLFWWindow*>(glfwGetWindowUserPointer(window));
	if (!handle) return;

	if(entered) {
		gWindowMouseEnterEvent event{};
		handle->callEvent(event);
	} else {
		gWindowMouseExitEvent event{};
		handle->callEvent(event);
	}
}

static void onMouseScroll(GLFWwindow* window, double xoffset, double yoffset) {
	auto handle = static_cast<gGLFWWindow*>(glfwGetWindowUserPointer(window));
	if (!handle) return;
	gMouseScrolledEvent event{static_cast<int>(xoffset), static_cast<int>(yoffset)};
	handle->callEvent(event);
}

static void glfwErrorCallback(int error, const char* description) {
	gLoge("gGLFWWindow") << "GLFW Error: " << error << ": " << description;
}

// Class stuff

gGLFWWindow::gGLFWWindow() {
	window = nullptr;
	cursor = new GLFWcursor*[7];
	scalex = 1.0f;
	scaley = 1.0f;
}

gGLFWWindow::~gGLFWWindow() {
}

void gGLFWWindow::initialize(int width, int height, int windowMode, bool isResizable) {
#ifdef WIN32
	HRESULT hr = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
#endif
	gBaseWindow::initialize(width, height, windowMode, isResizable);

	// Set error callback before glfwInit() if supported
#ifdef GLFW_VERSION_MAJOR
	#if (GLFW_VERSION_MAJOR > 3) || (GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR >= 0)
		glfwSetErrorCallback(glfwErrorCallback);
	#endif
#else
	// Fallback: try to set it anyway if version macros aren't available
	// This is safe as the function existed since GLFW 3.0
	int major, minor, rev;
	glfwGetVersion(&major, &minor, &rev);
	if (major > 3 || (major == 3 && minor >= 0)) {
		glfwSetErrorCallback(gGLFWWindow::glfwErrorCallback);
	}
#endif

#ifdef GVK_VULKAN
	// Hand GLFW the loader this engine is already linked against, rather than let it
	// search for one. Left to itself GLFW dlopens the loader by bare name, which
	// fails wherever it lives outside the default library search path - Homebrew's
	// /opt/homebrew/lib on macOS, for one. glfwVulkanSupported() would then report
	// false and the backend below would silently fall back to OpenGL. Must come
	// before glfwInit(), and is harmless for an OpenGL app: GLFW only ever uses the
	// pointer for Vulkan calls.
	glfwInitVulkanLoader(vkGetInstanceProcAddr);
#endif

	// Create glfw
	if (!glfwInit()) {
		gLoge("gGLFWWindow") << "Failed to initialize GLFW" << std::endl;
		return;
	}

	// --- Render backend selection (single, safe decision point) ---
	// The window is created before the renderer, so the backend has to be final
	// here; otherwise the window and the renderer could disagree.
	bool usevulkan = (appmanager != nullptr && appmanager->getRenderEngine() == G_RENDERER_VK);
#if !defined(GLIST_HAS_VULKAN)
	if(usevulkan) {
		gLoge("gGLFWWindow") << "Vulkan backend requested but Vulkan development support "
								"was not available when GlistEngine was built. Falling back to OpenGL." << std::endl;
		usevulkan = false;
		appmanager->setRenderEngine(G_RENDERER_GL);
	}
#endif
	if (usevulkan && !glfwVulkanSupported()) {
		gLoge("gGLFWWindow") << "Vulkan backend requested but not available at runtime "
								"(is the Vulkan loader installed?). Falling back to OpenGL." << std::endl;
		usevulkan = false;
		if (appmanager != nullptr) appmanager->setRenderEngine(G_RENDERER_GL);
	}

	// Configure glfw
	if (usevulkan) {
		// Vulkan renders to a surface it creates itself, so the window must not
		// carry an OpenGL context.
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	} else {
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if TARGET_OS_OSX
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //case_mac
#endif
	}

	// All hints available at https://www.glfw.org/docs/latest/window.html#window_hints

	// Create window
	int currentrefreshrate = 60;
    if(windowMode == G_WINDOWMODE_GAME) {
    	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    	if (!mode) {
    		gLoge("gGLFWWindow") << "Failed to get video mode for primary monitor" << std::endl;
    		glfwTerminate();
    		return;
    	}
    	width = mode->width;
    	height = mode->height;
    	currentrefreshrate = mode->refreshRate;
    } else if(windowMode == G_WINDOWMODE_FULLSCREEN || windowMode == G_WINDOWMODE_FULLSCREENGUIAPP) {
    	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    	glfwWindowHint(GLFW_RESIZABLE, isResizable);
    } else {
    	glfwWindowHint(GLFW_RESIZABLE, isResizable);
    }
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    window = glfwCreateWindow(width, height, title.c_str(),
			(windowMode == G_WINDOWMODE_GAME?glfwGetPrimaryMonitor():NULL), NULL);

	if(window == nullptr) {
		gLoge("gGLFWWindow") << "Failed to create GLFW window" << std::endl;
	    glfwTerminate();
	    return;
	}
    currentwindow = window;

	if(windowMode == G_WINDOWMODE_GAME) {
	   	GLFWmonitor* monitor = glfwGetWindowMonitor(window);
	   	if (monitor) {
	   		glfwSetWindowMonitor(window, monitor, 0, 0, width, height, currentrefreshrate);
	   	}
	   	// GL-only call; skipped under Vulkan where the window has no GL context.
	   	if (!usevulkan) glViewport(0, 0, width, height);
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

#ifndef EMSCRIPTEN
	// Load and set window icon
	GLFWimage images[1];
	std::string iconpath = gGetImagesDir() + "appicon/icon.png";
	images[0].pixels = stbi_load(iconpath.c_str(), &images[0].width, &images[0].height, 0, 4); //rgba channels
	if (images[0].pixels) {
		glfwSetWindowIcon(window, 1, images);
		stbi_image_free(images[0].pixels);
	} else {
		gLogw("gGLFWWindow") << "Failed to load window icon from " << iconpath << std::endl;
	}
#endif

	// Create cursors
	cursor[CURSOR_ARROW] = glfwCreateStandardCursor(0x00036001);
	cursor[CURSOR_IBEAM] = glfwCreateStandardCursor(0x00036002);
	cursor[CURSOR_CROSSHAIR] = glfwCreateStandardCursor(0x00036003);
	cursor[CURSOR_HAND] = glfwCreateStandardCursor(0x00036004);
	cursor[CURSOR_HRESIZE] = glfwCreateStandardCursor(0x00036005);
	cursor[CURSOR_VRESIZE] = glfwCreateStandardCursor(0x00036006);
	cursor[CURSOR_CUSTOM] = nullptr;

	if (cursor[0]) {
		glfwSetCursor(window, cursor[0]);
	}

	// OpenGL context management and GLEW are meaningless under Vulkan; that
	// window has no client API at all.
	if (!usevulkan) {
		glfwMakeContextCurrent(window);
		glfwSwapInterval(vsync ? 1 : 0);
		glewExperimental = GL_TRUE;
		GLenum glewError = glewInit();
		if (glewError != GLEW_OK) {
			gLoge("gGLFWWindow") << "Failed to initialize GLEW: " << glewGetErrorString(glewError) << std::endl;
			glfwTerminate();
			return;
		}
	}

//	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
//	    std::cout << "Failed to initialize GLAD" << std::endl;
//	    return -1;
//	}

	glfwSetWindowUserPointer(window, this);

	// glViewport is an OpenGL call, so it is meaningless - and unsafe, since a
	// GLFW_NO_API window has no current context - under Vulkan. The Vulkan backend
	// sets its viewport through the pipeline/command buffer instead.
	if (!usevulkan) {
		// Window specs to OpenGL
		glViewport(0, 0, width, height);
	}

	// Notify OpenGL if the window size changed
	glfwSetFramebufferSizeCallback(window, onFramebufferResize);
	glfwSetCharCallback(window, onCharInput);
	glfwSetKeyCallback(window, onKey);
	glfwSetCursorPosCallback(window, onMouseMove);
	glfwSetMouseButtonCallback(window, onMouseButton);
	glfwSetCursorEnterCallback(window, onMouseEnter);
	glfwSetScrollCallback(window, onMouseScroll);
	glfwSetWindowFocusCallback(window, onWindowFocus);
	glfwSetWindowContentScaleCallback(window, onScaleChange);
	glfwSetJoystickCallback(onJoystick);

    for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; ++jid) {
        if (glfwJoystickPresent(jid)) {
            onJoystick(jid, GLFW_CONNECTED);
        }
    }
	setCursorMode(CURSORMODE_NORMAL);
}

bool gGLFWWindow::getShouldClose() {
	return glfwWindowShouldClose(window);
}

void gGLFWWindow::update() {
	G_PROFILE_ZONE_SCOPED_N("gGLFWWindow::update()");
	// End window drawing. A GLFW_NO_API window (Vulkan) owns no buffers to swap,
	// and asking GLFW to swap them raises GLFW_NO_WINDOW_CONTEXT. The GL error
	// checks are skipped along with it: without a context every glGetError call
	// reports GL_INVALID_OPERATION, which would flood the log in debug builds.
	if(appmanager == nullptr || appmanager->getRenderEngine() != G_RENDERER_VK) {
		G_CHECK_GL(glfwSwapBuffers(window));
		G_CHECK_GL(glfwPollEvents());
	} else {
		glfwPollEvents();
	}
}

void gGLFWWindow::close() {
	// Clean up cursors
	for (int i = 0; i < 7; i++) {
		if (cursor[i]) {
			glfwDestroyCursor(cursor[i]);
			cursor[i] = nullptr;
		}
	}
	delete[] cursor;
	cursor = nullptr;

	// Deallocate glfw resources
	glfwTerminate();
}

bool gGLFWWindow::supportsVulkan() const {
#ifdef GVK_VULKAN
	return window != nullptr && glfwVulkanSupported() == GLFW_TRUE;
#else
	return false;
#endif
}

void gGLFWWindow::getVulkanInstanceExtensions(std::vector<const char*>& extensions) const {
#ifdef GVK_VULKAN
	uint32_t count = 0;
	const char** names = glfwGetRequiredInstanceExtensions(&count);
	if(names != nullptr) extensions.insert(extensions.end(), names, names + count);
#else
	(void)extensions;
#endif
}

bool gGLFWWindow::createVulkanSurface(void* instance, void* surface) {
#ifdef GVK_VULKAN
	if(window == nullptr || instance == nullptr || surface == nullptr) return false;
	return glfwCreateWindowSurface(*static_cast<VkInstance*>(instance), window, nullptr,
			static_cast<VkSurfaceKHR*>(surface)) == VK_SUCCESS;
#else
	(void)instance;
	(void)surface;
	return false;
#endif
}

void gGLFWWindow::setVsync(bool vsync) {
	gBaseWindow::setVsync(vsync);
	// glfwSwapInterval controls the current OpenGL context. Vulkan windows are
	// created with GLFW_NO_API and synchronise presentation through the swapchain
	// present mode instead, so calling it there raises GLFW_NO_CURRENT_CONTEXT.
	//
	// That other path has to actually be taken, though. Leaving it out is what kept
	// the Vulkan backend on FIFO whatever the game asked for - vsynced, and so
	// capped at the display's refresh rate while the OpenGL build of the same game
	// ran unlocked. It made the two look far apart on a frame counter when the
	// difference was that one of them was waiting for the monitor.
	if(window != nullptr && glfwGetWindowAttrib(window, GLFW_CLIENT_API) != GLFW_NO_API) {
		glfwSwapInterval(vsync ? 1 : 0);
	} else if(gRenderObject::getRenderer() != nullptr) {
		gRenderObject::getRenderer()->setVsync(vsync);
	}
}

void gGLFWWindow::setCursor(int cursorNo) {
	if (cursorNo >= 0 && cursorNo < 7 && cursor[cursorNo]) {
		glfwSetCursor(window, cursor[cursorNo]);
	}
}

void gGLFWWindow::setCursorMode(gCursorMode cursorMode) {
	cursormode = cursorMode;
	switch (cursorMode) {
	case CURSORMODE_NORMAL: {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		break;
	}
	case CURSORMODE_HIDDEN: {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		break;
	}
	case CURSORMODE_DISABLED: {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		break;
	}
	case CURSORMODE_RELATIVE: {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		setCursorPos(width / 2.0f, height / 2.0f);
		break;
	}
	}

	if (glfwRawMouseMotionSupported()) {
		// Raw input is only enabled when relative mouse is set
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, cursormode == CURSORMODE_RELATIVE);
	}
}

void gGLFWWindow::setCursorPos(int x, int y) {
	glfwSetCursorPos(window, x / scalex, y / scaley);
}

void gGLFWWindow::setCustomCursor(gImage& image, int hotspotX, int hotspotY){
	int w = image.getWidth();
	int h = image.getHeight();
	unsigned char* pixels = image.getImageData();

	if(!pixels || w <= 0 || h <= 0){
		gLogw("gGLFWWindow") << "Failed to set custom cursor: invalid gImage" << std::endl;
		return;
	}

	GLFWimage img;
	img.width = w;
	img.height = h;
	img.pixels = pixels;
	GLFWcursor* newcursor = glfwCreateCursor(&img, hotspotX, hotspotY);

	if(!newcursor){
		gLoge("gGLFWWindow") << "Failed to create custom cursor" << std::endl;
		return;
	}
	
	if(cursor[CURSOR_CUSTOM]){
		glfwDestroyCursor(cursor[CURSOR_CUSTOM]);
	}

	cursor[CURSOR_CUSTOM] = newcursor;
	setCursor(CURSOR_CUSTOM);
}

void gGLFWWindow::setClipboardString(std::string text) {
	glfwSetClipboardString(window, text.c_str());
}

std::string gGLFWWindow::getClipboardString() {
	const char* clipboardText = glfwGetClipboardString(window);
	return clipboardText ? std::string(clipboardText) : std::string("");
}

void gGLFWWindow::setWindowSize(int width, int height) {
	if(window != nullptr) {
		glfwSetWindowSize(window, width, height);
		onFramebufferResize(window, width, height);
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

void gGLFWWindow::setIcon(std::string pngFullpath) {
	GLFWimage images[1];
	std::string iconpath = pngFullpath;
	images[0].pixels = stbi_load(iconpath.c_str(), &images[0].width, &images[0].height, 0, 4); //rgba channels
	if (images[0].pixels) {
		glfwSetWindowIcon(window, 1, images);
		stbi_image_free(images[0].pixels);
	} else {
		gLogw("gGLFWWindow") << "Failed to load window icon from " << iconpath << std::endl;
	}
}

void gGLFWWindow::setIcon(unsigned char* imageData, int w, int h) {
	if (imageData && w > 0 && h > 0) {
		GLFWimage images[1];
		images[0].width = w;
		images[0].height = h;
		images[0].pixels = imageData;
		glfwSetWindowIcon(window, 1, images);
	}
}

void gGLFWWindow::setTitle(const std::string& windowTitle) {
	title = windowTitle;
	if (window) {
		glfwSetWindowTitle(window, windowTitle.c_str());
	}
}

bool gGLFWWindow::isJoystickPresent(int joystickId) {
	return glfwJoystickPresent(joystickId);
}

bool gGLFWWindow::isGamepadButtonPressed(int joystickId, int buttonId) {
#ifndef EMSCRIPTEN // todo
	GLFWgamepadstate gpstate;
	if (glfwGetGamepadState(joystickId, &gpstate) == GLFW_TRUE) {
		return gpstate.buttons[buttonId];
	}
#endif
	return false;
}

const float* gGLFWWindow::getJoystickAxes(int joystickId, int* axisCountPtr) {
	return glfwGetJoystickAxes(joystickId, axisCountPtr);
}

void gGLFWWindow::setScale(float x, float y) {
	scalex = x;
	scaley = y;
	// Update framebuffer size since scale changed
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	gWindowScaleChangedEvent event{width, height, x, y};
	callEvent(event);
	setSize(width, height);
}
