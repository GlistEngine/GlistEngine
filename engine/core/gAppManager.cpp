/*
 * gAppManager.cpp
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

#include "gAppManager.h"
#include "gBaseComponent.h"
#include "gBasePlugin.h"
#include "gBaseApp.h"
#include "gCanvasManager.h"
#include "gGUIFrame.h"

#include <thread>

// Platform specific window implementation
#if defined(WIN32) || defined(LINUX) || defined(APPLE)
#include "gGLFWWindow.h"
#elif defined(ANDROID)
#include "gAndroidWindow.h"
#endif

void gStartEngine(gBaseApp* baseApp, const std::string& appName, int windowMode, int width, int height, bool isResizable) {
    gStartEngine(baseApp, appName, windowMode, width, height, G_SCREENSCALING_AUTO, width, height, isResizable);
}

void gStartEngine(gBaseApp* baseApp, const std::string& appName, int windowMode, int unitWidth, int unitHeight, int screenScaling, int width, int height, bool isResizable) {
    if(windowMode == G_WINDOWMODE_NONE) windowMode = G_WINDOWMODE_APP;
    if(windowMode == G_WINDOWMODE_FULLSCREENGUIAPP || windowMode == G_WINDOWMODE_GUIAPP) screenScaling = G_SCREENSCALING_NONE;
#ifndef ANDROID
    gAppManager manager(appName, baseApp, width, height, windowMode, unitWidth, unitHeight, screenScaling, isResizable, G_LOOPMODE_NORMAL);
	manager.initialize();
	manager.setup();
	manager.loop();
	manager.stop();
#else
    new gAppManager(appName, baseApp, width, height, windowMode, unitWidth, unitHeight, screenScaling, isResizable, G_LOOPMODE_NORMAL);
#endif
}

void gStartEngine(gBaseApp* baseApp, const std::string& appName, int loopMode) {
#ifndef ANDROID
	gAppManager manager(appName, baseApp, 0, 0, G_WINDOWMODE_NONE, 0, 0, G_SCREENSCALING_NONE, false, loopMode);
    manager.initialize();
	manager.setup();
	manager.loop();
    manager.stop();
#else
	throw std::runtime_error("windowless android applications are not supported yet!");
#endif
}

int pow(int x, int p) {
    int i = 1;
    for (int j = 1; j <= p; j++) i *= x;
    return i;
}

gAppManager* appmanager = nullptr;

gAppManager::gAppManager(const std::string& appName, gBaseApp *baseApp, int width, int height,
                         int windowMode, int unitWidth, int unitHeight, int screenScaling,
                         bool isResizable, int loopMode) : appname(appName), app(baseApp), width(width), height(height),
                                                                 windowmode(windowMode), unitwidth(unitWidth), unitheight(unitHeight), screenscaling(screenScaling),
                                                                 isresizable(isResizable), loopmode(loopMode) {
    appmanager = this;
	if(windowMode != G_WINDOWMODE_NONE) {
		canvasmanager = new gCanvasManager();
	} else {
		canvasmanager = nullptr;
	}
	guimanager = nullptr;
    initialized = false;
    initializedbefore = false;
    iscanvasset = false;
    isrunning = false;
    setupcomplete = false;
#ifdef ANDROID
	isrendering = false;
#else
	isrendering = true;
#endif
    // Mouse
    ismouseentered = false;
    mousebuttonstate = 0;
    for(int i = 0; i < maxmousebuttonnum; i++) {
        mousebuttonpressed[i] = false;
    }
    targetframerate = 60;
    updateTime();
    starttime = AppClock::now();
    endtime = starttime;
    deltatime = AppClockDuration(0);
    totaltime = 0;
    totalupdates = 0;
    totaldraws = 0;
    iswindowfocused = false;
#ifdef ANDROID
    deviceorientation = DEVICEORIENTATION_PORTRAIT;
    olddeviceorientation = DEVICEORIENTATION_PORTRAIT;
#endif

    // Joystick
    isjoystickenabled = false;
    for(int i = 0; i < maxjoysticknum; i++) {
        for(int j = 0; j < maxjoystickbuttonnum; j++) joystickbuttonstate[i][j] = false;
        joystickconnected[i] = false;
    }
    joystickaxecount = 0;
    eventhandler = G_BIND_FUNCTION(onEvent);

	if (appname.empty()) {
		appname = "GlistApp";
	}
	if(windowMode != G_WINDOWMODE_NONE) {
		usewindow = true;
#ifdef ANDROID
		window = new gAndroidWindow();
#else
		window = new gGLFWWindow();
#endif
		window->setEventHandler(eventhandler);
		window->setTitle(appname);
	} else {
		usewindow = false;
		window = nullptr;
		setTargetFramerate(INT_MAX);
	}
}

gAppManager::~gAppManager() {
    delete canvasmanager;
    delete guimanager;
    delete window;
    gRenderObject::destroyRenderer();
}

void gAppManager::setup() {
    if(setupcomplete) {
        return;
    }
    app->setup();
    setupcomplete = true;
}

void gAppManager::initialize() {
    if(initialized || loopmode == G_LOOPMODE_NONE) {
		app->start();
        return;
    }
	deltatime = AppClockDuration(0);

	if(usewindow) {
		window->initialize(width, height, windowmode, isresizable);
		// Update size
		width = window->getWidth();
		height = window->getHeight();
		if(unitwidth == 0) {
			unitwidth = this->width;
		}
		if(unitheight == 0) {
			unitheight = this->height;
		}
		// Create renderer
		gRenderObject::createRenderer();
		// Update renderer dimensions
		gBaseCanvas::setScreenSize(width, height);
		gBaseCanvas::setUnitScreenSize(unitwidth, unitheight);
		gBaseCanvas::setScreenScaling(screenscaling);
		// Create managers if not created
		if(!guimanager) {
			guimanager = new gGUIManager(app, width, height);
		}
	}
    initialized = true;
    initializedbefore = true;
    app->start();
}

void gAppManager::loop() {
    if(loopmode == G_LOOPMODE_NONE) {
        return;
    }

#ifdef DEBUG
    if(usewindow) {
		assert(window);
        assert(gRenderObject::getRenderer());
    }
#endif
    //gLogi("gAppManager") << "starting loop";
    isrunning = true;
	starttime = AppClock::now();

    while (isrunning && (!usewindow || !window->getShouldClose())) {
        // Delta time calculations
        endtime = AppClock::now();
        deltatime = endtime - starttime;
        totaltime += deltatime.count();
        starttime = endtime;

        tick();

        if(totaltime >= 1'000'000'000) {
            totaltime = 0;
            totalupdates = 0;
            totaldraws = 0;
        }

        if(!usewindow || !window->vsync) {
            double sleepTime = (targettimestep - (AppClock::now() - starttime)).count() / 1'000'000'000.0;
            if(sleepTime > 0) {
                preciseSleep(sleepTime);
            }
        }
    }
    //gLogi("gAppManager") << "stopping loop";
    app->stop();
    gRenderObject::destroyRenderer();
    if(usewindow) {
        window->close();
    }
    initialized = false;
}

void gAppManager::stop() {
    isrunning = false;
}

std::string gAppManager::getAppName() {
	return appname;
}

gCanvasManager* gAppManager::getCanvasManager() {
	return canvasmanager;
}

gGUIManager* gAppManager::getGUIManager() {
	return guimanager;
}

int gAppManager::getLoopMode() {
	return loopmode;
}

bool gAppManager::isWindowFocused() {
	return iswindowfocused;
}

void gAppManager::setWindowSize(int width, int height) {
	window->setWindowSize(width, height);
}

void gAppManager::setWindowResizable(bool isResizable) {
	window->setWindowResizable(isResizable);
}

void gAppManager::setWindowSizeLimits(int minWidth, int minHeight, int maxWidth, int maxHeight) {
	window->setWindowSizeLimits(minWidth, minHeight, maxWidth, maxHeight);
}

void gAppManager::setScreenSize(int width, int height) {
    canvasmanager->getCurrentCanvas()->setScreenSize(width, height);
    if(iscanvasset) canvasmanager->getCurrentCanvas()->windowResized(width, height);
    if(iscanvasset && guimanager->isframeset) guimanager->windowResized(width, height);
}

void gAppManager::setCurrentCanvas(gBaseCanvas* canvas) {
    canvasmanager->setCurrentCanvas(canvas);
    iscanvasset = true;
}

gBaseCanvas* gAppManager::getCurrentCanvas() {
    return canvasmanager->getCurrentCanvas();
}

void gAppManager::setTargetFramerate(int framerate) {
    targetframerate = framerate;
	updateTime();
}

int gAppManager::getTargetFramerate() {
    return targetframerate;
}

int gAppManager::getFramerate() {
    return (uint32_t)(1'000'000'000 / deltatime.count());
}

void gAppManager::enableVsync() {
    window->setVsync(false);
}

void gAppManager::disableVsync() {
    window->setVsync(false);
}

void gAppManager::setCurrentGUIFrame(gGUIFrame *guiFrame) {
    guimanager->setCurrentFrame(guiFrame);
}

double gAppManager::getElapsedTime() {
    return deltatime.count() / 1'000'000'000.0;
}

void gAppManager::setClipboardString(const std::string &clipboard) {
    window->setClipboardString(clipboard);
}

std::string gAppManager::getClipboardString() {
    return window->getClipboardString();
}

void gAppManager::setCursor(int cursorId) {
    window->setCursor(cursorId);
}

void gAppManager::setCursorMode(int cursorMode) {
    window->setCursorMode(cursorMode);
}

bool gAppManager::isJoystickConnected(int joystickId) {
    if(joystickId >= maxjoysticknum) return false;
    return joystickconnected[joystickId];
}

int gAppManager::getJoystickAxesCount(int joystickId) {
    return joystickaxecount;
}

const float* gAppManager::getJoystickAxes(int joystickId) {
    if(joystickId >= maxjoysticknum || !isjoystickenabled || !joystickconnected[joystickId]) {
        return nullptr;
    }
    return window->getJoystickAxes(joystickId, &joystickaxecount);
}

bool gAppManager::isGamepadButtonPressed(int joystickId, int buttonId) {
    if(joystickId >= maxjoysticknum || !isjoystickenabled || !joystickconnected[joystickId]) {
        return false;
    }
    return window->isGamepadButtonPressed(joystickId, buttonId);
}

#ifdef ANDROID
void gAppManager::setDeviceOrientation(DeviceOrientation orientation) {
    gAndroidUtil::setDeviceOrientation(orientation);
}
#endif

void gAppManager::tick() {
    totalupdates++;
    if(!usewindow) {
        app->update();
        for(int i = 0; i < gBaseComponent::usedcomponents.size(); i++) {
            gBaseComponent::usedcomponents[i]->update();
        }
		executeQueue();
        return;
    }

    // todo joystick
    if(canvasmanager) canvasmanager->update();
    if(guimanager) guimanager->update();
    app->update();
    for(int i = 0; i < gBaseComponent::usedcomponents.size(); i++) {
        gBaseComponent::usedcomponents[i]->update();
    }
    for(int i = 0; i < gBasePlugin::usedplugins.size(); i++) {
        gBasePlugin::usedplugins[i]->update();
    }

    gBaseCanvas* canvas = canvasmanager->getCurrentCanvas();
    if(canvas) {
        canvas->update();
    }

    if(isrendering) {
        if(canvas) {
            canvas->clearBackground();
            for (int i = 0; i < renderpassnum; i++) {
                renderpassno = i;
                canvas->draw();
            }
        }

		if(guimanager) guimanager->draw();
        totaldraws++;
    }
	if(usewindow) {
		window->update();
	}
    if(!window->isRendering() && isrendering) {
        isrendering = false; // If window has lost the context, we should stop rendering.
#ifdef ANDROID
		app->pause();
#endif
    }
	executeQueue();
}


void gAppManager::onEvent(gEvent& event) {
    if(event.ishandled) return;

    gEventDispatcher dispatcher(event);
    dispatcher.dispatch<gWindowResizeEvent>(G_BIND_FUNCTION(onWindowResizedEvent));
    dispatcher.dispatch<gCharTypedEvent>(G_BIND_FUNCTION(onCharTypedEvent));
    dispatcher.dispatch<gKeyPressedEvent>(G_BIND_FUNCTION(onKeyPressedEvent));
    dispatcher.dispatch<gKeyReleasedEvent>(G_BIND_FUNCTION(onKeyReleasedEvent));
    dispatcher.dispatch<gMouseMovedEvent>(G_BIND_FUNCTION(onMouseMovedEvent));
    dispatcher.dispatch<gMouseButtonPressedEvent>(G_BIND_FUNCTION(onMouseButtonPressedEvent));
    dispatcher.dispatch<gMouseButtonReleasedEvent>(G_BIND_FUNCTION(onMouseButtonReleasedEvent));
    dispatcher.dispatch<gWindowMouseEnterEvent>(G_BIND_FUNCTION(onWindowMouseEnterEvent));
    dispatcher.dispatch<gWindowMouseExitEvent>(G_BIND_FUNCTION(onWindowMouseExitEvent));
    dispatcher.dispatch<gMouseScrolledEvent>(G_BIND_FUNCTION(onMouseScrolledEvent));
    dispatcher.dispatch<gWindowFocusEvent>(G_BIND_FUNCTION(onWindowFocusEvent));
    dispatcher.dispatch<gWindowLoseFocusEvent>(G_BIND_FUNCTION(onWindowLoseFocusEvent));
    dispatcher.dispatch<gJoystickConnectEvent>(G_BIND_FUNCTION(onJoystickConnectEvent));
    dispatcher.dispatch<gJoystickDisconnectEvent>(G_BIND_FUNCTION(onJoystickDisconnectEvent));
#ifdef ANDROID
    dispatcher.dispatch<gAppPauseEvent>(G_BIND_FUNCTION(onAppPauseEvent));
    dispatcher.dispatch<gAppResumeEvent>(G_BIND_FUNCTION(onAppResumeEvent));
    dispatcher.dispatch<gDeviceOrientationChangedEvent>(G_BIND_FUNCTION(onDeviceOrientationChangedEvent));
#endif
    if(canvasmanager && canvasmanager->getCurrentCanvas()) canvasmanager->getCurrentCanvas()->onEvent(event);
    // todo pass event to app and plugins
}

bool gAppManager::onWindowResizedEvent(gWindowResizeEvent& event) {
    if(!canvasmanager || !canvasmanager->getCurrentCanvas() || !initialized) return true;
    setScreenSize(event.getWidth(), event.getHeight());
#ifdef ANDROID
    int orientation = deviceorientation % 2;
    int oldorientation = olddeviceorientation % 2;
    if(gRenderer::getScreenScaling() >= G_SCREENSCALING_AUTO && oldorientation != orientation) {
        int width = gRenderObject::getRenderer()->getUnitWidth();
        int height = gRenderObject::getRenderer()->getUnitHeight();
        if(oldorientation == orientation) {
            gRenderer::setUnitScreenSize(width, height);
        } else {
            gRenderer::setUnitScreenSize(height, width);
        }
    }
    olddeviceorientation = deviceorientation;
#endif
    return false;
}

bool gAppManager::onCharTypedEvent(gCharTypedEvent& event) {
    if(guimanager->isframeset) guimanager->charPressed(event.getCharacter());
    for (gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->charPressed(event.getCharacter());
    canvasmanager->getCurrentCanvas()->charPressed(event.getCharacter());
    return false;
}

bool gAppManager::onKeyPressedEvent(gKeyPressedEvent& event) {
    if (!canvasmanager->getCurrentCanvas()) return true;
    if(guimanager->isframeset) guimanager->keyPressed(event.getKeyCode());
    for (gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->keyPressed(event.getKeyCode());
    canvasmanager->getCurrentCanvas()->keyPressed(event.getKeyCode());
    return false;
}

bool gAppManager::onKeyReleasedEvent(gKeyReleasedEvent& event) {
    if (!canvasmanager->getCurrentCanvas()) return true;
    if(guimanager->isframeset) guimanager->keyReleased(event.getKeyCode());
    for (gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->keyReleased(event.getKeyCode());
    canvasmanager->getCurrentCanvas()->keyReleased(event.getKeyCode());
    return false;
}

bool gAppManager::onMouseMovedEvent(gMouseMovedEvent& event) {
    if (!canvasmanager->getCurrentCanvas()) return true;
    int xpos = event.getX();
    int ypos = event.getY();
    if (gRenderer::getScreenScaling() > G_SCREENSCALING_NONE) {
        xpos = gRenderer::scaleX(event.getX());
        ypos = gRenderer::scaleY(event.getY());
    }
    if (mousebuttonstate) {
        if(guimanager->isframeset) guimanager->mouseDragged(xpos, ypos, mousebuttonstate);
        for(gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mouseDragged(xpos, ypos, mousebuttonstate);
        canvasmanager->getCurrentCanvas()->mouseDragged(xpos, ypos, mousebuttonstate);
    } else {
        if(guimanager->isframeset) guimanager->mouseMoved(xpos, ypos);
        for(gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mouseMoved(xpos, ypos);
        canvasmanager->getCurrentCanvas()->mouseMoved(xpos, ypos);
    }
    return false;
}

bool gAppManager::onMouseButtonPressedEvent(gMouseButtonPressedEvent& event) {
    if (!canvasmanager->getCurrentCanvas()) return true;
    mousebuttonpressed[event.getMouseButton()] = true;
    mousebuttonstate |= pow(2, event.getMouseButton() + 1);
    int xpos = event.getX();
    int ypos = event.getY();
    if (gRenderer::getScreenScaling() > G_SCREENSCALING_NONE) {
        xpos = gRenderer::scaleX(event.getX());
        ypos = gRenderer::scaleY(event.getY());
    }
    if(guimanager->isframeset) guimanager->mousePressed(xpos, ypos, event.getMouseButton());
    for(gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mousePressed(xpos, ypos, event.getMouseButton());
    canvasmanager->getCurrentCanvas()->mousePressed(xpos, ypos, event.getMouseButton());
    return false;
}

bool gAppManager::onMouseButtonReleasedEvent(gMouseButtonReleasedEvent& event) {
    if (!canvasmanager->getCurrentCanvas()) return true;
    mousebuttonpressed[event.getMouseButton()] = false;
    mousebuttonstate &= ~pow(2, event.getMouseButton() + 1);
    int xpos = event.getX();
    int ypos = event.getY();
    if(gRenderer::getScreenScaling() > G_SCREENSCALING_NONE) {
        xpos = gRenderer::scaleX(event.getX());
        ypos = gRenderer::scaleY(event.getY());
    }
    if(guimanager->isframeset) guimanager->mouseReleased(xpos, ypos, event.getMouseButton());
    for(gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mouseReleased(xpos, ypos, event.getMouseButton());
    canvasmanager->getCurrentCanvas()->mouseReleased(xpos, ypos, event.getMouseButton());
    return false;
}

bool gAppManager::onWindowMouseEnterEvent(gWindowMouseEnterEvent& event) {
    if(!canvasmanager->getCurrentCanvas()) return true;
    ismouseentered = true;
    for(gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mouseEntered();
    canvasmanager->getCurrentCanvas()->mouseEntered();
    return false;
}

bool gAppManager::onWindowMouseExitEvent(gWindowMouseExitEvent& event) {
    if(!canvasmanager->getCurrentCanvas()) return true;
    ismouseentered = false;
    for(gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mouseExited();
    canvasmanager->getCurrentCanvas()->mouseExited();
    return false;
}

bool gAppManager::onMouseScrolledEvent(gMouseScrolledEvent& event) {
    if (!canvasmanager->getCurrentCanvas()) return true;
    if(guimanager->isframeset) guimanager->mouseScrolled(event.getOffsetX(), event.getOffsetY());
    for (gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mouseScrolled(event.getOffsetX(), event.getOffsetY());
    canvasmanager->getCurrentCanvas()->mouseScrolled(event.getOffsetX(), event.getOffsetY());
    return false;
}

bool gAppManager::onWindowFocusEvent(gWindowFocusEvent& event) {
    iswindowfocused = true;
    return false;
}

bool gAppManager::onWindowLoseFocusEvent(gWindowLoseFocusEvent& event) {
    iswindowfocused = false;
    return false;
}

bool gAppManager::onJoystickConnectEvent(gJoystickConnectEvent& event) {
    if(event.getJoystickId() >= maxjoysticknum) return true;

    if(event.isGamepad()) {
        joystickconnected[event.getJoystickId()] = true;
        isjoystickenabled = true;
    }
    if (!canvasmanager->getCurrentCanvas()) return true;
    canvasmanager->getCurrentCanvas()->joystickConnected(event.getJoystickId(), event.isGamepad(), true);
    return false;
}

bool gAppManager::onJoystickDisconnectEvent(gJoystickDisconnectEvent& event) {
    if(event.getJoystickId() >= maxjoysticknum) return true;

    bool wasgamepad = joystickconnected[event.getJoystickId()];
    joystickconnected[event.getJoystickId()] = false;
    isjoystickenabled = false;
    // If at least one joystick is connected
    for(int i = 0; i < maxjoysticknum; i++) {
        if(joystickconnected[event.getJoystickId()]) {
            isjoystickenabled = true;
            break;
        }
    }
    if (!canvasmanager->getCurrentCanvas()) return true;
    canvasmanager->getCurrentCanvas()->joystickConnected(event.getJoystickId(), wasgamepad, false);
    return false;
}

#ifdef ANDROID
bool gAppManager::onAppPauseEvent(gAppPauseEvent& event) {
    submitToMainThread([this]() {
		if(!isrendering) {
			return;
		}
        isrendering = false;
        app->pause();
    });
    return false;
}

bool gAppManager::onAppResumeEvent(gAppResumeEvent& event) {
    submitToMainThread([this]() {
		if(isrendering) {
			return;
		}
        isrendering = true;
        app->resume();
    });
    return false;
}

bool gAppManager::onDeviceOrientationChangedEvent(gDeviceOrientationChangedEvent& event) {
    deviceorientation = event.getOrientation();
    if(usewindow && canvasmanager && canvasmanager->getCurrentCanvas()) {
        canvasmanager->getCurrentCanvas()->onDeviceOrientationChange(event.getOrientation());
    }
    gWindowResizeEvent resizeevent{width, height};
    eventhandler(resizeevent);
    return false;
}

#endif

void gAppManager::updateTime() {
	targettimestep = AppClockDuration(1'000'000'000 / (targetframerate + 1));
}

void gAppManager::submitToMainThread(std::function<void()> fn) {
    // todo use scoped_lock when switched to c++17
    std::unique_lock<std::mutex> lock(mainthreadqueuemutex);
    mainthreadqueue.emplace_back(fn);
}

void gAppManager::executeQueue() {
    // todo use scoped_lock when switched to c++17
    std::unique_lock<std::mutex> lock(mainthreadqueuemutex);
    for (auto& func : mainthreadqueue) {
        func();
    }
    mainthreadqueue.clear();
}

void gAppManager::preciseSleep(double seconds) {
    double estimate = 5e-3;
    double mean = 5e-3;
    double m2 = 0;
    double count = 1;
    double observed = 0;
    double delta = 0;
    double stddev = 0;

    while (seconds > estimate) {
        AppClockTimePoint start = AppClock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        AppClockTimePoint end = AppClock::now();

        observed = (end - start).count() / 1'000'000'000.0;
        seconds -= observed;

        count++;
        delta = observed - mean;
        mean += delta / count;
        m2 += delta * (observed - mean);
        stddev = std::sqrt(m2 / (count - 1));
        estimate = mean + stddev;
    }

    // spin lock
    AppClockTimePoint start = AppClock::now();
    while ((AppClock::now() - start).count() / 1'000'000'000.0 < seconds);
}


#ifdef ANDROID
extern "C" {

JNIEXPORT void JNICALL Java_dev_glist_android_lib_GlistNative_onCreate(JNIEnv *env, jclass clazz) {
    //gLogi("GlistNative") << "onCreate";
    androidMain();
#ifdef DEBUG
    assert(appmanager); // appmanager should not be null after androidMain();
#endif
}

JNIEXPORT void JNICALL Java_dev_glist_android_lib_GlistNative_onDestroy(JNIEnv *env, jclass clazz) {
    //gLogi("GlistNative") << "onDestroy";
    delete appmanager;
    appmanager = nullptr;
}

std::unique_ptr<std::thread> thread;
JNIEXPORT void JNICALL Java_dev_glist_android_lib_GlistNative_onStart(JNIEnv *env, jclass clazz) {
    gLogi("GlistNative") << "onStart";
    if(thread) {
        throw std::runtime_error("cannot call onStart without calling onStop first");
    }
    thread = std::make_unique<std::thread>([]() {
        appmanager->initialize();
        appmanager->setup();
        appmanager->loop();
    });
}

JNIEXPORT void JNICALL Java_dev_glist_android_lib_GlistNative_onStop(JNIEnv *env, jclass clazz) {
    //gLogi("GlistNative") << "onStop";
    if(appmanager) {
        appmanager->stop();
    }
    if(thread) {
        thread->join(); // wait for shutdown
        thread = nullptr;
    }
}

JNIEXPORT void JNICALL Java_dev_glist_android_lib_GlistNative_onPause(JNIEnv *env, jclass clazz) {
    //gLogi("GlistNative") << "onPause";
    if(appmanager) {
        gAppPauseEvent event{};
        appmanager->getEventHandler()(event);
    }
}

JNIEXPORT void JNICALL Java_dev_glist_android_lib_GlistNative_onResume(JNIEnv *env, jclass clazz) {
    //gLogi("GlistNative") << "onResume";
    if(appmanager) {
        gAppResumeEvent event{};
        appmanager->getEventHandler()(event);
    }
}
}
#endif
