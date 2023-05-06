/*
 * gAppManager.cpp
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

#include <thread>
#include "gAppManager.h"
#include "gGLFWWindow.h"
#include "gBaseApp.h"
#include "gBaseCanvas.h"
#include "gCanvasManager.h"
#include "gBaseComponent.h"
#include "gBasePlugin.h"
#if defined(WIN32) || defined(LINUX) || defined(APPLE)
#include <GLFW/glfw3.h>
#endif


void gStartEngine(gBaseApp* baseApp, const std::string& appName, int windowMode, int width, int height) {
	int wmode = windowMode;
	if(wmode == G_WINDOWMODE_NONE) wmode = G_WINDOWMODE_APP;

	gAppManager appmanager;
	gGLFWWindow gbwindow;
	gbwindow.setAppManager(&appmanager);
	std::string appname = appName;
	if (appname == "") appname = "GlistApp";
	gbwindow.setTitle(appname);
	appmanager.setWindow(&gbwindow);
	baseApp->setAppManager(&appmanager);
	int screenwidth = width, screenheight = height;
#if defined(WIN32) || defined(LINUX) || defined(APPLE)
	if (wmode == G_WINDOWMODE_GAME || wmode == G_WINDOWMODE_FULLSCREEN || wmode == G_WINDOWMODE_FULLSCREENGUIAPP) {
		glfwInit();
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		screenwidth = mode->width;
		screenheight = mode->height;
		glfwTerminate();
	}
#endif
	int screenscaling = G_SCREENSCALING_AUTO;
	if(wmode == G_WINDOWMODE_FULLSCREENGUIAPP || wmode == G_WINDOWMODE_GUIAPP) screenscaling = G_SCREENSCALING_NONE;
	appmanager.runApp(appName, baseApp, screenwidth, screenheight, wmode, width, height, screenscaling, G_LOOPMODE_NORMAL);
}

void gStartEngine(gBaseApp* baseApp, const std::string& appName, int windowMode, int unitWidth, int unitHeight, int screenScaling, int width, int height) {
	int wmode = windowMode;
	if(wmode == G_WINDOWMODE_NONE) wmode = G_WINDOWMODE_APP;

	gAppManager appmanager;
	gGLFWWindow gbwindow;
	gbwindow.setAppManager(&appmanager);
	std::string appname = appName;
	if (appname == "") appname = "GlistApp";
	gbwindow.setTitle(appname);
	appmanager.setWindow(&gbwindow);
	baseApp->setAppManager(&appmanager);
	int screenwidth = width, screenheight = height;
#if defined(WIN32) || defined(LINUX) || defined(APPLE)
	if (wmode == G_WINDOWMODE_GAME || wmode == G_WINDOWMODE_FULLSCREEN || wmode == G_WINDOWMODE_FULLSCREENGUIAPP) {
		glfwInit();
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		screenwidth = mode->width;
		screenheight = mode->height;
		glfwTerminate();
	}
#endif
	appmanager.runApp(appName, baseApp, screenwidth, screenheight, wmode, unitWidth, unitHeight, screenScaling, G_LOOPMODE_NORMAL);
}

void gStartEngine(gBaseApp* baseApp, const std::string& appName, int loopMode) {
	gAppManager appmanager;
	std::string appname = appName;
	if (appname == "") appname = "GlistApp";
	baseApp->setAppManager(&appmanager);
	appmanager.runApp(appName, baseApp, 0, 0, G_WINDOWMODE_NONE, 0, 0, 0, loopMode);
}


gAppManager::gAppManager() {
	appname = "";
	window = nullptr;
	app = nullptr;
	windowmode = G_WINDOWMODE_GAME;
	usewindow = true;
	loopmode = G_LOOPMODE_NORMAL;
	loopalways = true;
	canvas = nullptr;
	canvasmanager = nullptr;
	guimanager = nullptr;
	ismouseentered = false;
	buttonpressed[0] = false;
	buttonpressed[1] = false;
	buttonpressed[2] = false;
	pressed = 0;
	framerate = 60;
	timestepnano = AppClockDuration(1'000'000'000 / (framerate + 1));
	starttime = AppClock::now();
	endtime = starttime;
	deltatime = AppClockDuration(0);
	lag = AppClockDuration(0);
	elapsedtime = 0;
	updates = 0;
	draws = 0;
	uci = 0;
	ucj = 0;
	mpi = 0;
	mpj = 0;
	upi = 0;
	upj = 0;
	canvasset = false;
	iswindowfocused = false;
	isgamepadenabled = false;
	gpbuttonstate = false;
	for(int i = 0; i < maxgamepadnum; i++) {
		for(int j = 0; j < gamepadbuttonnum; j++) gamepadbuttonstate[i][j] = false;
		gamepadconnected[i] = false;
	}
	joystickhatcount = 0;
	joystickaxecount = 0;
}

void gAppManager::runApp(const std::string& appName, gBaseApp *baseApp, int width, int height, int windowMode, int unitWidth, int unitHeight, int screenScaling, int loopMode) {
	// Create app
	appname = appName;
	app = baseApp;
	windowmode = windowMode;
	if(windowmode == G_WINDOWMODE_NONE) usewindow = false;
	if(!usewindow) {
		framerate = INT_MAX;
		timestepnano = AppClockDuration(1'000'000'000 / (framerate + 1));
		starttime = AppClock::now();
		endtime = starttime;
		deltatime = AppClockDuration(0);
		lag = AppClockDuration(0);
	}

	// Create loop mode
	loopmode = loopMode;
	if(loopmode == G_LOOPMODE_NONE) loopalways = false;

	// Create window
	if(usewindow) {
		window->initialize(width, height, windowMode);

		// Update the width and height in case it was changed by the operating system or the window implementation
		width = window->getWidth();
		height = window->getHeight();

		canvasmanager = new gCanvasManager();
		guimanager = new gGUIManager(app, width, height);
	}
	for(int i = 0; i < maxgamepadnum; i++) {
		gamepadconnected[i] = glfwJoystickPresent(i);
		if(gamepadconnected[i]) isgamepadenabled = true;
	}

	// Run app
	app->setup();

	// Exit if console app does not loop
	if(!loopalways) return;

	// Add a temporary empty canvas to prevent errors
	if(usewindow) {
		gBaseCanvas *tempcanvas = canvasmanager->getTempCanvas();
		tempcanvas->setScreenSize(width, height);
		tempcanvas->setUnitScreenSize(unitWidth, unitHeight);
		tempcanvas->setScreenScaling(screenScaling);
	}

	// Main loop
	while(!usewindow || !window->getShouldClose()) {
		// Delta time calculations
		endtime = AppClock::now();
		deltatime = endtime - starttime;
		elapsedtime += deltatime.count();
		starttime = endtime;

		internalUpdate();

		if(!usewindow || !window->vsync) {
			/* Less precision, but lower CPU usage for non vsync */
			sleeptime = (timestepnano - (AppClock::now() - starttime)).count() / 1e9;
			if (sleeptime > 0.0f) {
				preciseSleep(sleeptime);
			}
			/* Much more precise method, but eats up CPU */
//			lag += deltatime;
//			while(lag >= timestepnano) {
//				lag -= timestepnano;
//				internalUpdate();
//			}
		}
	}

	if(usewindow) window->close();
}

void gAppManager::internalUpdate() {
	if(!usewindow) {
		app->update();
		for (uci = 0; uci < gBaseComponent::usedcomponents.size(); uci++) gBaseComponent::usedcomponents[uci]->update();
		return;
	}

	canvasmanager->update();
	if(isgamepadenabled && iswindowfocused) {
		GLFWgamepadstate gpstate;
		for(uci = 0; uci < maxgamepadnum; uci++) {
			if(!gamepadconnected[uci]) continue;

			glfwGetGamepadState(uci, &gpstate);
			for(ucj = 0; ucj < gamepadbuttonnum; ucj++) {
				gpbuttonstate = false;
				if(gpstate.buttons[ucj]) gpbuttonstate = true;
				if(gpbuttonstate != gamepadbuttonstate[uci][ucj]) {
					if(gpbuttonstate) canvasmanager->getCurrentCanvas()->gamepadButtonPressed(uci, ucj);
					else canvasmanager->getCurrentCanvas()->gamepadButtonReleased(uci, ucj);
				}
				gamepadbuttonstate[uci][ucj] = gpbuttonstate;
			}
		}
	}
	if(guimanager->isframeset) guimanager->update();
	app->update();
	for (uci = 0; uci < gBaseComponent::usedcomponents.size(); uci++) gBaseComponent::usedcomponents[uci]->update();
	for (upi = 0; upi < gBasePlugin::usedplugins.size(); upi++) gBasePlugin::usedplugins[upi]->update();
	canvas = canvasmanager->getCurrentCanvas();
	if (canvas != nullptr) {
		canvas->update();
		updates++;
		canvas->clearBackground();
		for (upj = 0; upj < renderpassnum; upj++) {
			renderpassno = upj;
			canvas->draw();
			draws++;
		}
	}

	if(guimanager->isframeset) guimanager->draw();
	window->update();

	if (elapsedtime >= 1'000'000'000){
		elapsedtime = 0;
		updates = 0;
		draws = 0;
	}
}

void gAppManager::preciseSleep(double seconds) {
	t_estimate = 5e-3;
    t_mean = 5e-3;
    t_m2 = 0;
    t_count = 1;

    while (seconds > t_estimate) {
        auto t_start = AppClock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        auto end = AppClock::now();

        t_observed = (end - t_start).count() / 1e9;
        seconds -= t_observed;

        ++t_count;
        t_delta = t_observed - t_mean;
        t_mean += t_delta / t_count;
        t_m2   += t_delta * (t_observed - t_mean);
        t_stddev = sqrt(t_m2 / (t_count - 1));
        t_estimate = t_mean + t_stddev;
    }

    // spin lock
    auto start = AppClock::now();
    while ((AppClock::now() - start).count() / 1e9 < seconds);
}

void gAppManager::setWindow(gBaseWindow *baseWindow) {
	window = baseWindow;
}

int gAppManager::getWindowMode() {
	return windowmode;
}

void gAppManager::setCursor(int cursorId) {
	window->setCursor(cursorId);
}

void gAppManager::setCursorMode(int cursorMode) {
	window->setCursorMode(cursorMode);
}

gCanvasManager* gAppManager::getCanvasManager() {
	return canvasmanager;
}

void gAppManager::setCurrentCanvas(gBaseCanvas *baseCanvas) {
	canvasmanager->setCurrentCanvas(baseCanvas);
	canvasset = true;
}

gBaseCanvas* gAppManager::getCurrentCanvas() {
	return canvas;
}

void gAppManager::setScreenSize(int width, int height) {
	canvas->setScreenSize(width, height);
	if(canvasset) canvasmanager->getCurrentCanvas()->windowResized(width, height);
	if(canvasset && guimanager->isframeset) guimanager->windowResized(width, height);
}

void gAppManager::setFramerate(int targetFramerate) {
	framerate = targetFramerate;
	timestepnano = AppClockDuration(1'000'000'000 / (framerate + 1));
}

std::string gAppManager::getAppName() {
	return appname;
}


gGUIManager* gAppManager::getGUIManager() {
	return guimanager;
}

void gAppManager::setCurrentGUIFrame(gGUIFrame *guiFrame) {
	guimanager->setCurrentFrame(guiFrame);
}

gGUIFrame* gAppManager::getCurrentGUIFrame() {
	return guimanager->getCurrentFrame();
}

void gAppManager::enableVsync() {
	window->enableVsync(true);
}

void gAppManager::disableVsync() {
	window->enableVsync(false);
}

int gAppManager::getFramerate() {
    return (long long)(1'000'000'000 / deltatime.count());
}

double gAppManager::getElapsedTime() {
	return deltatime.count() / 1'000'000'000.0f;
}

void gAppManager::setClipboardString(std::string text) {
	window->setClipboardString(text);
}

std::string gAppManager::getClipboardString() {
	return window->getClipboardString();
}

int gAppManager::getLoopMode() {
	return loopmode;
}

void gAppManager::onCharEvent(unsigned int key) {
#if defined(WIN32) || defined(LINUX) || defined(APPLE)
	if(guimanager->isframeset) guimanager->charPressed(key);
	for (upi = 0; upi < gBasePlugin::usedplugins.size(); upi++) gBasePlugin::usedplugins[upi]->charPressed(key);
	canvasmanager->getCurrentCanvas()->charPressed(key);
#endif
}

void gAppManager::onKeyEvent(int key, int action) {
#if defined(WIN32) || defined(LINUX) || defined(APPLE)
	switch(action) {
	case GLFW_RELEASE:
		if(guimanager->isframeset) guimanager->keyReleased(key);
		for (upi = 0; upi < gBasePlugin::usedplugins.size(); upi++) gBasePlugin::usedplugins[upi]->keyReleased(key);
		canvasmanager->getCurrentCanvas()->keyReleased(key);
		break;
	case GLFW_PRESS:
		if(guimanager->isframeset) guimanager->keyPressed(key);
		for (upi = 0; upi < gBasePlugin::usedplugins.size(); upi++) gBasePlugin::usedplugins[upi]->keyPressed(key);
		canvasmanager->getCurrentCanvas()->keyPressed(key);
		break;
	}
#endif
}

void gAppManager::onMouseMoveEvent(double xpos, double ypos) {
	if (!canvasmanager->getCurrentCanvas()) return;
	if (gRenderer::getScreenScaling() > G_SCREENSCALING_NONE) {
		xpos = gRenderer::scaleX(xpos);
		ypos = gRenderer::scaleY(ypos);
	}
	if (pressed) {
		if(guimanager->isframeset) guimanager->mouseDragged(xpos, ypos, pressed);
		for (upi = 0; upi < gBasePlugin::usedplugins.size(); upi++) gBasePlugin::usedplugins[upi]->mouseDragged(xpos, ypos, pressed);
		canvasmanager->getCurrentCanvas()->mouseDragged(xpos, ypos, pressed);
	} else {
		if(guimanager->isframeset) guimanager->mouseMoved(xpos, ypos);
		for (upi = 0; upi < gBasePlugin::usedplugins.size(); upi++) gBasePlugin::usedplugins[upi]->mouseMoved(xpos, ypos);
		canvasmanager->getCurrentCanvas()->mouseMoved(xpos, ypos);
	}
}

void gAppManager::onMouseButtonEvent(int button, int action, double xpos, double ypos) {
	if (!canvasmanager->getCurrentCanvas()) return;
#if defined(WIN32) || defined(LINUX) || defined(APPLE)
	switch(action) {
	case GLFW_PRESS:
		buttonpressed[button] = true;
		pressed |= myPow(2, button + 1);
		if (gRenderer::getScreenScaling() > G_SCREENSCALING_NONE) {
			xpos = gRenderer::scaleX(xpos);
			ypos = gRenderer::scaleY(ypos);
		}
		if(guimanager->isframeset) guimanager->mousePressed(xpos, ypos, button);
		for (upi = 0; upi < gBasePlugin::usedplugins.size(); upi++) gBasePlugin::usedplugins[upi]->mousePressed(xpos, ypos, button);
		canvasmanager->getCurrentCanvas()->mousePressed(xpos, ypos, button);
		break;
	case GLFW_RELEASE:
		buttonpressed[button] = false;
		pressed &= ~myPow(2, button + 1);
		if (gRenderer::getScreenScaling() > G_SCREENSCALING_NONE) {
			xpos = gRenderer::scaleX(xpos);
			ypos = gRenderer::scaleY(ypos);
		}
		if(guimanager->isframeset) guimanager->mouseReleased(xpos, ypos, button);
		for (upi = 0; upi < gBasePlugin::usedplugins.size(); upi++) gBasePlugin::usedplugins[upi]->mouseReleased(xpos, ypos, button);
		canvasmanager->getCurrentCanvas()->mouseReleased(xpos, ypos, button);
		break;
	}
#endif
}

int gAppManager::myPow (int x, int p) {
	  mpi = 1;
	  for (mpj = 1; mpj <= p; mpj++)  mpi *= x;
	  return mpi;
}

void gAppManager::onMouseEnterEvent(int entered) {
	if (!canvasmanager->getCurrentCanvas()) return;
	if (entered) {
		ismouseentered = true;
		for (upi = 0; upi < gBasePlugin::usedplugins.size(); upi++) gBasePlugin::usedplugins[upi]->mouseEntered();
		canvasmanager->getCurrentCanvas()->mouseEntered();
	} else {
		ismouseentered = false;
		for (upi = 0; upi < gBasePlugin::usedplugins.size(); upi++) gBasePlugin::usedplugins[upi]->mouseExited();
		canvasmanager->getCurrentCanvas()->mouseExited();
	}
}

void gAppManager::onMouseScrollEvent(double xoffset, double yoffset) {
	if (!canvasmanager->getCurrentCanvas()) return;
	if(guimanager->isframeset) guimanager->mouseScrolled(xoffset, yoffset);
	for (upi = 0; upi < gBasePlugin::usedplugins.size(); upi++) gBasePlugin::usedplugins[upi]->mouseScrolled(xoffset, yoffset);
	canvasmanager->getCurrentCanvas()->mouseScrolled(xoffset, yoffset);
}

void gAppManager::onWindowFocus(bool isFocused) {
	iswindowfocused = isFocused;
}

bool gAppManager::isWindowFocused() {
	return iswindowfocused;
}

bool gAppManager::isJoystickConnected(int jId) {
	return gamepadconnected[jId];
}

int gAppManager::getJoystickAxesCount(int jId) {
	return joystickaxecount;
}

const float* gAppManager::getJoystickAxes(int jId) {
	if(!isgamepadenabled) return nullptr;
	if(!gamepadconnected[jId]) return nullptr;

	return glfwGetJoystickAxes(jId, &joystickaxecount);
}

bool gAppManager::isGamepadEnabled() {
	return isgamepadenabled;
}

bool gAppManager::isGamepadButtonPressed(int gamepadId, int buttonId) {
	GLFWgamepadstate gpstate;
	glfwGetGamepadState(gamepadId, &gpstate);
	if(gpstate.buttons[buttonId]) return true;
	return false;
}

int gAppManager::getMaxGamepadNum() {
	return maxgamepadnum;
}

int gAppManager::getGamepadButtonNum() {
	return gamepadbuttonnum;
}

void gAppManager::onJoystickConnected(int jid, bool isGamepad, bool isConnected) {
	if (!canvasmanager->getCurrentCanvas()) return;
	if(jid >= maxgamepadnum) return;

	if(isGamepad) {
		gamepadconnected[jid] = true;
		isgamepadenabled = true;
	} else {
		gamepadconnected[jid] = false;
		bool gamepadenabledtemp = false;
		for(int i = 0; i < maxgamepadnum; i++) {
			if(gamepadconnected[jid]) {
				gamepadenabledtemp = true;
				break;
			}
		}
		isgamepadenabled = gamepadenabledtemp;
	}
	canvasmanager->getCurrentCanvas()->joystickConnected(jid, isGamepad, isConnected);
}
