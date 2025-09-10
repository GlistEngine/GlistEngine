/*
 * gAppManager.cpp
 *
 *  Created on: May 6, 2020
 *      Author: Noyan Culum
 */

#include "gAppManager.h"
#include "gBaseComponent.h"
#include "gBasePlugin.h"
#include "gBaseApp.h"
#include "gCanvasManager.h"
#include "gGUIFrame.h"

#include <thread>
#include "gGUIAppThread.h"
#include "gTracy.h"

// Platform specific window implementation
#if defined(WIN32) || defined(LINUX) || TARGET_OS_OSX
#include "gGLFWWindow.h"
#elif defined(ANDROID)
#include "gAndroidWindow.h"
#include "gAndroidCanvas.h"
#include "gAndroidApp.h"
#elif TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
#   include "gIOSWindow.h"
#   include "gIOSCanvas.h"
#   include "gIOSApp.h"
#   include "gIOSMain.h"
#endif

void gStartEngine(gBaseApp* baseApp, const std::string& appName, int windowMode, int width, int height, bool isResizable) {
    gStartEngine(baseApp, appName, windowMode, width, height, G_SCREENSCALING_AUTO, width, height, isResizable);
}

void gStartEngine(gBaseApp* baseApp, const std::string& appName, int windowMode, int unitWidth, int unitHeight, int screenScaling, int width, int height, bool isResizable) {
    if(windowMode == G_WINDOWMODE_NONE) windowMode = G_WINDOWMODE_APP;
    if(windowMode == G_WINDOWMODE_FULLSCREENGUIAPP || windowMode == G_WINDOWMODE_GUIAPP) screenScaling = G_SCREENSCALING_NONE;
#if TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
    ios_main(baseApp, appName.c_str(), windowMode, unitWidth, unitHeight, screenScaling, width, height, isResizable);
#elif defined(ANDROID)
    new gAppManager(appName, baseApp, width, height, windowMode, unitWidth, unitHeight, screenScaling, isResizable, G_LOOPMODE_NORMAL);
#else
    gAppManager manager(appName, baseApp, width, height, windowMode, unitWidth, unitHeight, screenScaling, isResizable, G_LOOPMODE_NORMAL);
	manager.runApp();
#endif
}

void gStartEngine(gBaseApp* baseApp, const std::string& appName, int loopMode) {
#if !(defined(GLIST_MOBILE))
	gAppManager manager(appName, baseApp, 0, 0, G_WINDOWMODE_NONE, 0, 0, G_SCREENSCALING_NONE, false, loopMode);
	manager.runApp();
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
	ansilocale = "";
#if defined(WIN32)
	ansilocale = setlocale(LC_ALL, ".ACP");
#endif
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
    guiappthread = nullptr;
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
	framerate = targetframerate;
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
    delayedresize = false;
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
#if defined(ANDROID)
		window = new gAndroidWindow();
#elif TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
        window = new gIOSWindow();
#else
		window = new gGLFWWindow();
#endif
		window->setEventHandler(eventhandler);
	} else {
		usewindow = false;
		window = nullptr;
		setTargetFramerate(INT_MAX);
	}

	isguiapp = false;
	if(windowMode == G_WINDOWMODE_GUIAPP || windowMode == G_WINDOWMODE_FULLSCREENGUIAPP) isguiapp = true;
	if(isguiapp) guiappthread = new gGUIAppThread(baseApp);
}

gAppManager::~gAppManager() {
	if(guiappthread) {
		guiappthread->stop();
//		delete guiappthread;
	}
    delete canvasmanager;
    delete guimanager;
    delete window;
    gRenderObject::destroyRenderer();
}

gGUIAppThread* gAppManager::getGUIAppThread() {
	return guiappthread;
}

void gAppManager::setup() {
    if(setupcomplete) {
        return;
    }
    app->setup();
    setupcomplete = true;
}

void gAppManager::runApp() {
	initialize();
	setup();
	if(isguiapp) guiappthread->start();
	loop();
	stop();
}

void gAppManager::initialize() {
    if(initialized || loopmode == G_LOOPMODE_NONE) {
		app->start();
        return;
    }
	deltatime = AppClockDuration(0);

	if(usewindow) {
		window->initialize(width, height, windowmode, isresizable);
		window->setTitle(appname);
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
			guimanager->getCurrentFrame()->getRenderer()->updateLights();
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
//        assert(renderer);
    }
#endif
    //gLogi("gAppManager") << "starting loop";
    isrunning = true;
	starttime = AppClock::now();

#ifdef ANDROID
	// This ensures the resize function is called before calling setup for the canvas when
	// device orientation is changed.
	while(delayedresize) {
		executeQueue();
	}
#endif

#if !(TARGET_OS_IPHONE || TARGET_OS_SIMULATOR)
    while (isrunning && (!usewindow || !window->getShouldClose())) {
        // Delta time calculations
        endtime = AppClock::now();
        deltatime = endtime - starttime;
        totaltime += deltatime.count();
        starttime = endtime;
		totalupdates++;

        tick();

        if(totaltime >= 1'000'000'000) {
        	framerate = totalupdates;
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
    if(usewindow) {
        window->close();
    }
    initialized = false;
#endif // !(TARGET_OS_IPHONE || TARGET_OS_SIMULATOR)
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

bool gAppManager::callEvent(gEvent& event) {
	eventhandler(event);
	return event.ishandled;
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
	G_PROFILE_ZONE_SCOPED_N("gAppManager::setScreenSize()");
	G_PROFILE_ZONE_VALUE(width);
	G_PROFILE_ZONE_VALUE(height);
    gRenderObject::setScreenSize(width, height);
    if(iscanvasset && canvasmanager->getCurrentCanvas()) canvasmanager->getCurrentCanvas()->windowResized(width, height);
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
    return framerate;
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

void gAppManager::setWindowIcon(std::string pngFullpath) {
	window->setIcon(pngFullpath);
}

void gAppManager::setWindowIcon(unsigned char* imageData, int w, int h) {
	window->setIcon(imageData, w, h);
}

void gAppManager::setWindowTitle(const std::string& windowTitle) {
	window->setTitle(windowTitle);
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

std::string gAppManager::getANSILocale() {
	return ansilocale;
}

#ifdef ANDROID
void gAppManager::setDeviceOrientation(DeviceOrientation orientation) {
	if(!isrunning) {
		gAndroidUtil::setDeviceOrientation(orientation);
		delayedresize = true;
	} else {
		gAndroidUtil::setDeviceOrientation(orientation);
	}
}
#endif

void gAppManager::tick() {
	G_PROFILE_FRAME_MARK();
    if(!usewindow) {
		G_PROFILE_ZONE_SCOPED_N("gAppManager::tick(): Non Window Update");
        app->update();
    	for (gBaseComponent*& component : gBaseComponent::usedcomponents) {
    		component->update();
    	}
		executeQueue();
        return;
    }

    // todo joystick
    if(canvasmanager) canvasmanager->update();
    if(guimanager) guimanager->update();
    if(!isguiapp) {
		G_PROFILE_ZONE_SCOPED_N("gAppManager::tick(): App Update");
    	app->update();
    }
    {
		G_PROFILE_ZONE_SCOPED_N("gAppManager::tick(): Components Update");
    	for (gBaseComponent*& component : gBaseComponent::usedcomponents) {
    		component->update();
    	}
    }
    {
		G_PROFILE_ZONE_SCOPED_N("gAppManager::tick(): Plugins Update");
    	for (gBasePlugin*& component : gBasePlugin::usedplugins) {
    		component->update();
    	}
    }

    gBaseCanvas* canvas = nullptr;
    if(!isguiapp) {
		if(canvasmanager) canvas = canvasmanager->getCurrentCanvas();
		if(canvas) {
			G_PROFILE_ZONE_SCOPED_N("gAppManager::tick(): Canvas Update");
			canvas->update();
		}
    }

    if(isrendering) {
    	if(!isguiapp) {
			if(canvas) {
				canvas->clearBackground();
				for (int i = 0; i < renderpassnum; i++) {
					G_PROFILE_ZONE_SCOPED_N("gGUIManager::tick(): Render Pass");
					G_PROFILE_ZONE_VALUE(i);
					renderpassno = i;
					gBaseCanvas::getRenderer()->updateScene();
					canvas->draw();
				}
			}
    	}

		if(guimanager) {
			guimanager->draw();
		}
        totaldraws++;
    }
	if(usewindow) {
		window->update();
	}
    if(!window->isRendering() && isrendering) {
        isrendering = false; // If window has lost the context, we should stop rendering.
#ifdef ANDROID
		if (auto* androidapp = dynamic_cast<gAndroidApp*>(app)) {
			androidapp->pause();
		}
		if (canvasmanager && getCurrentCanvas()) {
			if (auto* androidcanvas = dynamic_cast<gAndroidCanvas*>(getCurrentCanvas())) {
				androidcanvas->pause();
			}
		}
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
#if defined(GLIST_MOBILE)
    dispatcher.dispatch<gAppPauseEvent>(G_BIND_FUNCTION(onAppPauseEvent));
    dispatcher.dispatch<gAppResumeEvent>(G_BIND_FUNCTION(onAppResumeEvent));
    dispatcher.dispatch<gDeviceOrientationChangedEvent>(G_BIND_FUNCTION(onDeviceOrientationChangedEvent));
    dispatcher.dispatch<gTouchEvent>(G_BIND_FUNCTION(onTouchEvent));
#endif
    if(canvasmanager && getCurrentCanvas()) getCurrentCanvas()->onEvent(event);
    // todo pass event to app and plugins
}

bool gAppManager::onWindowResizedEvent(gWindowResizeEvent& event) {
    if(!canvasmanager || !initialized || (!getCurrentCanvas() && !canvasmanager->getTempCanvas())) {
        return true;
    }
    setScreenSize(event.getWidth(), event.getHeight());
#ifdef ANDROID
    delayedresize = false;
    if(gRenderer::getScreenScaling() >= G_SCREENSCALING_AUTO && olddeviceorientation != deviceorientation) {
		DeviceOrientation orientation = deviceorientation;
		DeviceOrientation oldorientation = olddeviceorientation;
		// Normalize values
		if(orientation == DEVICEORIENTATION_REVERSE_PORTRAIT) {
			orientation = DEVICEORIENTATION_PORTRAIT;
		} else if(orientation == DEVICEORIENTATION_REVERSE_LANDSCAPE) {
			orientation = DEVICEORIENTATION_LANDSCAPE;
		}
		if(oldorientation == DEVICEORIENTATION_REVERSE_PORTRAIT) {
			oldorientation = DEVICEORIENTATION_PORTRAIT;
		} else if(oldorientation == DEVICEORIENTATION_REVERSE_LANDSCAPE) {
			oldorientation = DEVICEORIENTATION_LANDSCAPE;
		}

		bool swapdimensions = oldorientation != orientation;
		if((orientation != DEVICEORIENTATION_PORTRAIT && orientation != DEVICEORIENTATION_LANDSCAPE) ||
			(oldorientation != DEVICEORIENTATION_PORTRAIT && oldorientation != DEVICEORIENTATION_LANDSCAPE)) {
			// If this orientation is not known, we should not do anything
			swapdimensions = false;
		}

		// Orientation changed, we should swap height and width
		if(swapdimensions) {
			int unitwidth = renderer->getUnitWidth();
			int unitheight = renderer->getUnitHeight();
			// Swap width and height values
			gRenderer::setUnitScreenSize(unitheight, unitwidth);
		}
    }
    olddeviceorientation = deviceorientation;
#endif
    return false;
}

bool gAppManager::onCharTypedEvent(gCharTypedEvent& event) {
//	if (!canvasmanager || !getCurrentCanvas()) return true;
    if(guimanager->isframeset) guimanager->charPressed(event.getCharacter());
    for (gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->charPressed(event.getCharacter());
    if(canvasmanager && getCurrentCanvas()) getCurrentCanvas()->charPressed(event.getCharacter());
    return false;
}

bool gAppManager::onKeyPressedEvent(gKeyPressedEvent& event) {
//    if (!canvasmanager || !getCurrentCanvas()) return true;
    if(guimanager->isframeset) guimanager->keyPressed(event.getKeyCode());
    for (gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->keyPressed(event.getKeyCode());
    if(canvasmanager && getCurrentCanvas()) getCurrentCanvas()->keyPressed(event.getKeyCode());
    return false;
}

bool gAppManager::onKeyReleasedEvent(gKeyReleasedEvent& event) {
//    if (!canvasmanager || !getCurrentCanvas()) return true;
    if(guimanager->isframeset) guimanager->keyReleased(event.getKeyCode());
    for (gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->keyReleased(event.getKeyCode());
    if(canvasmanager && getCurrentCanvas()) canvasmanager->getCurrentCanvas()->keyReleased(event.getKeyCode());
    return false;
}

bool gAppManager::onMouseMovedEvent(gMouseMovedEvent& event) {
//    if (!canvasmanager || !getCurrentCanvas()) return true;
    int xpos = event.getX();
    int ypos = event.getY();
    if (gRenderer::getScreenScaling() > G_SCREENSCALING_NONE) {
        xpos = gRenderer::scaleX(event.getX());
        ypos = gRenderer::scaleY(event.getY());
    }
    if (mousebuttonstate) {
        if(guimanager->isframeset) guimanager->mouseDragged(xpos, ypos, mousebuttonstate);
        for(gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mouseDragged(xpos, ypos, mousebuttonstate);
        if(canvasmanager && getCurrentCanvas()) canvasmanager->getCurrentCanvas()->mouseDragged(xpos, ypos, mousebuttonstate);
    } else {
        if(guimanager->isframeset) guimanager->mouseMoved(xpos, ypos);
        for(gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mouseMoved(xpos, ypos);
        if(canvasmanager && getCurrentCanvas()) canvasmanager->getCurrentCanvas()->mouseMoved(xpos, ypos);
    }
    return false;
}

bool gAppManager::onMouseButtonPressedEvent(gMouseButtonPressedEvent& event) {
//    if (!canvasmanager || !getCurrentCanvas()) return true;
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
    if(canvasmanager && getCurrentCanvas()) canvasmanager->getCurrentCanvas()->mousePressed(xpos, ypos, event.getMouseButton());
    return false;
}

bool gAppManager::onMouseButtonReleasedEvent(gMouseButtonReleasedEvent& event) {
//    if (!canvasmanager || !getCurrentCanvas()) return true;
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
    if(canvasmanager && getCurrentCanvas()) canvasmanager->getCurrentCanvas()->mouseReleased(xpos, ypos, event.getMouseButton());
    return false;
}

bool gAppManager::onWindowMouseEnterEvent(gWindowMouseEnterEvent& event) {
//    if(!canvasmanager || !getCurrentCanvas()) return true;
    ismouseentered = true;
    for(gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mouseEntered();
    if(canvasmanager && getCurrentCanvas()) canvasmanager->getCurrentCanvas()->mouseEntered();
    return false;
}

bool gAppManager::onWindowMouseExitEvent(gWindowMouseExitEvent& event) {
//    if(!canvasmanager || !getCurrentCanvas()) return true;
    ismouseentered = false;
    for(gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mouseExited();
    if(canvasmanager && getCurrentCanvas()) canvasmanager->getCurrentCanvas()->mouseExited();
    return false;
}

bool gAppManager::onMouseScrolledEvent(gMouseScrolledEvent& event) {
//    if (!canvasmanager || !getCurrentCanvas()) return true;
    if(guimanager->isframeset) guimanager->mouseScrolled(event.getOffsetX(), event.getOffsetY());
    for (gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mouseScrolled(event.getOffsetX(), event.getOffsetY());
    if(canvasmanager && getCurrentCanvas()) canvasmanager->getCurrentCanvas()->mouseScrolled(event.getOffsetX(), event.getOffsetY());
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

    joystickconnected[event.getJoystickId()] = true;
    isjoystickenabled = true;
    if (!canvasmanager || !getCurrentCanvas()) return true;
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
    if (!canvasmanager || !getCurrentCanvas()) return true;
    canvasmanager->getCurrentCanvas()->joystickConnected(event.getJoystickId(), wasgamepad, false);
    return false;
}

#if TARGET_OS_IPHONE || TARGET_OS_SIMULATOR

void gAppManager::iosLoop()
{
    if(!(isrunning && (!usewindow || !window->getShouldClose())))
    {
        app->stop();
        if(usewindow) {
            window->close();
        }
        initialized = false;
    }
    
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
    //gLogi("gAppManager") << "stopping loop";
}

#endif // TARGET_OS_IPHONE || TARGET_OS_SIMULATOR

#if defined(GLIST_MOBILE)

bool gAppManager::onAppPauseEvent(gAppPauseEvent& event) {
    submitToMainThread([this]() {
		if(!isrendering) {
			return;
		}
        isrendering = false;
		if (
#if defined(ANDROID)
            auto* mobileapp = dynamic_cast<gAndroidApp*>(app)
#elif TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
			auto* mobileapp = static_cast<gIOSApp*>(app)
#endif
        ) {
            mobileapp->pause();
		}
		if(canvasmanager && getCurrentCanvas()) {
			if (
#if defined(ANDROID)
                auto* mobilecanvas = dynamic_cast<gAndroidCanvas*>(getCurrentCanvas())
#elif TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
                auto* mobilecanvas = static_cast<gIOSCanvas*>(getCurrentCanvas())
#endif
            ) {
				mobilecanvas->pause();
			}
		}
	});
    return false;
}

bool gAppManager::onAppResumeEvent(gAppResumeEvent& event) {
    submitToMainThread([this]() {
		if(isrendering) {
			return;
		}
        isrendering = true;
		if (
#if defined(ANDROID)
            auto* mobileapp = dynamic_cast<gAndroidApp*>(app)
#elif TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
            auto* mobileapp = static_cast<gIOSApp*>(app)
#endif
        ) {
            mobileapp->resume();
		}
		if(canvasmanager && getCurrentCanvas()) {
			if (
#if defined(ANDROID)
                auto* mobilecanvas = dynamic_cast<gAndroidCanvas*>(getCurrentCanvas())
#elif TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
                auto* mobilecanvas = static_cast<gIOSCanvas*>(getCurrentCanvas())
#endif
            ) {
				mobilecanvas->resume();
			}
		}
	});
    return false;
}

bool gAppManager::onDeviceOrientationChangedEvent(gDeviceOrientationChangedEvent& event) {
    deviceorientation = event.getOrientation();
    if(canvasmanager && getCurrentCanvas()) {
		if (
#if defined(ANDROID)
            auto* mobilecanvas = dynamic_cast<gAndroidCanvas*>(getCurrentCanvas())
#elif TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
            auto* mobilecanvas = static_cast<gIOSCanvas*>(getCurrentCanvas())
#endif
        ) {
			mobilecanvas->deviceOrientationChanged(event.getOrientation());
		}
    }
    return false;
}

bool gAppManager::onTouchEvent(gTouchEvent& event) {
	if(canvasmanager && getCurrentCanvas()) {
		if (
#if defined(ANDROID)
            auto* mobilecanvas = dynamic_cast<gAndroidCanvas*>(getCurrentCanvas())
#elif TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
            auto* mobilecanvas = static_cast<gIOSCanvas*>(getCurrentCanvas())
#endif
        ) {
			if (event.getAction() == ACTIONTYPE_POINTER_DOWN || (event.getInputCount() == 1 && event.getAction() == ACTIONTYPE_DOWN)) {
				int inputindex = event.getActionIndex();
				TouchInput& input = event.getInputs()[inputindex];
				int x = input.x;
				int y = input.y;
				if(gRenderer::getScreenScaling() > G_SCREENSCALING_NONE) {
					x = gRenderer::scaleX(x);
					y = gRenderer::scaleY(y);
				}
				mobilecanvas->touchPressed(x, y, input.fingerid);
			} else if (event.getAction() == ACTIONTYPE_POINTER_UP || (event.getInputCount() == 1 && event.getAction() == ACTIONTYPE_UP)) {
				int inputindex = event.getActionIndex();
				TouchInput& input = event.getInputs()[inputindex];
				int x = input.x;
				int y = input.y;
				if(gRenderer::getScreenScaling() > G_SCREENSCALING_NONE) {
					x = gRenderer::scaleX(x);
					y = gRenderer::scaleY(y);
				}
				mobilecanvas->touchReleased(x, y, input.fingerid);
			} else if (event.getAction() == ACTIONTYPE_MOVE) {
				int inputindex = event.getActionIndex();
				TouchInput& input = event.getInputs()[inputindex];
				int x = input.x;
				int y = input.y;
				if(gRenderer::getScreenScaling() > G_SCREENSCALING_NONE) {
					x = gRenderer::scaleX(x);
					y = gRenderer::scaleY(y);
				}
				mobilecanvas->touchMoved(x, y, input.fingerid);
			}
		}
	}
	return false;
}

#endif // defined(GLIST_MOBILE)

void gAppManager::updateTime() {
	targettimestep = AppClockDuration(1'000'000'000 / (targetframerate + 1));
}

void gAppManager::submitToMainThread(std::function<void()> fn) {
    std::unique_lock<std::mutex> lock(mainthreadqueuemutex);
    mainthreadqueue.emplace_back(fn);
}

void gAppManager::executeQueue() {
	G_PROFILE_ZONE_SCOPED_N("gAppManager::executeQueue()");
    std::unique_lock<std::mutex> lock(mainthreadqueuemutex);
    for (auto& func : mainthreadqueue) {
        func();
    }
    mainthreadqueue.clear();
}

void gAppManager::preciseSleep(double seconds) {
	G_PROFILE_ZONE_SCOPED_N("gAppManager::preciseSleep()");
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
