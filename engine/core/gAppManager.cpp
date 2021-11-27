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
#include "gBasePlugin.h"
#if defined(WIN32) || defined(LINUX)
#include <GLFW/glfw3.h>
#endif

using namespace std::chrono_literals;

void gStartEngine(gBaseApp* baseApp, std::string appName, int windowMode, int width, int height) {
	gAppManager appmanager;
	gGLFWWindow gbwindow;
	gbwindow.setAppManager(&appmanager);
	if (appName == "") appName = "GlistApp";
	gbwindow.setTitle(appName);
	appmanager.setWindow(&gbwindow);
	baseApp->setAppManager(&appmanager);
	appmanager.runApp(appName, baseApp, width, height, windowMode, width, height, gRenderer::SCREENSCALING_AUTO);
}

void gStartEngine(gBaseApp* baseApp, std::string appName, int windowMode, int width, int height, int screenScaling, int unitWidth, int unitHeight) {
	gAppManager appmanager;
	gGLFWWindow gbwindow;
	gbwindow.setAppManager(&appmanager);
	if (appName == "") appName = "GlistApp";
	gbwindow.setTitle(appName);
	appmanager.setWindow(&gbwindow);
	baseApp->setAppManager(&appmanager);
	appmanager.runApp(appName, baseApp, width, height, windowMode, unitWidth, unitHeight, screenScaling);
}


gAppManager::gAppManager() {
	appname = "";
	window = nullptr;
	app = nullptr;
	canvas = nullptr;
	canvasmanager = nullptr;
	guimanager = nullptr;
	ismouseentered = false;
	buttonpressed[0] = false;
	buttonpressed[1] = false;
	buttonpressed[2] = false;
	pressed = 0;
	desiredframerate = 60;
	desiredframetime = std::chrono::nanoseconds(16ms);
	mpi = 0;
	mpj = 0;
	upi = 0;
	upj = 0;
}

gAppManager::~gAppManager() {
}

void gAppManager::runApp(std::string appName, gBaseApp *baseApp, int width, int height, int windowMode, int unitWidth, int unitHeight, int screenScaling) {
	appname = appName;
	app = baseApp;

	// Create window
	window->initialize(width, height, windowMode);

	canvasmanager = new gCanvasManager();
	guimanager = new gGUIManager(app);

	// Run app
	app->setup();
//	if (canvasmanager->getTempCanvas() != nullptr) {
//		canvasmanager->getTempCanvas()->setup(); // Commented out because was invoking first canvas's setup 2 times in the app launch
//	}

	gBaseCanvas *tempcanvas = canvasmanager->getTempCanvas();
	tempcanvas->setScreenSize(width, height);
	tempcanvas->setUnitScreenSize(unitWidth, unitHeight);
	tempcanvas->setScreenScaling(screenScaling);

	std::chrono::nanoseconds lag(0ns);
	using Clock = std::chrono::steady_clock;
	auto timestart = Clock::now();
	auto currenttime = Clock::now();
	// Main loop
	while(!window->getShouldClose()) {
		// calculate the elapsed time
		currenttime = Clock::now();
		auto frametime = currenttime - timestart;
		timestart = currenttime;

		// calculate lag (accumulates with time)
		lag += std::chrono::duration_cast<std::chrono::nanoseconds>(frametime);

		/*
		 * If the last frame took longer than expected, for example;
		 *
		 * We want our frame to take exactly 1 second but the last frame took
		 * 2 seconds, that means we missed a frame in there. To accommodate that,
		 * in the while loop, if the lag time is more than the desired frame time,
		 * we first substract the desired time from the lag, and then update once.
		 * Now our lag became 1 second. Again we continue inside the while loop,
		 * substract desired frame time from lag, then update. Now, we have updated
		 * twice. One for the missed last frame, and one for our current frame.
		 *
		 * If the last frame took less than expected;
		 *
		 * For example if our computer ran fast and completed the frame in 0.5 seconds,
		 * while our desired frame time was 1 second, we don't update in that frame
		 * and just make the program render the stuff from the last frame. With that
		 * we make sure that we run an exact amount of frames per second with a
		 * fixed frame rate.
		 *
		 * We basically render as much as possible and limit the number of updates.
		 */
		while(lag >= desiredframetime) {
			lag -= desiredframetime;
			canvasmanager->update();
			if(guimanager->isframeset) guimanager->update();
			app->update();
			for (upi = 0; upi < gBasePlugin::usedplugins.size(); upi++) gBasePlugin::usedplugins[upi]->update();

			canvas = canvasmanager->getCurrentCanvas();
			if(canvas != nullptr) canvas->update();
		}
		if(canvas != nullptr) {
			for (upj = 0; upj < renderpassnum; upj++) {
				renderpassno = upj;
				canvas->clearBackground();
				canvas->draw();
			}
		}
		if(guimanager->isframeset) guimanager->draw();
		window->update();
	}

	window->close();
}

void gAppManager::setWindow(gBaseWindow *baseWindow) {
	window = baseWindow;
}

void gAppManager::setCursor(int cursorId) {
	window->setCursor(cursorId);
}

gCanvasManager* gAppManager::getCanvasManager() {
	return canvasmanager;
}

void gAppManager::setCurrentCanvas(gBaseCanvas *baseCanvas) {
	canvasmanager->setCurrentCanvas(baseCanvas);
}

gBaseCanvas* gAppManager::getCurrentCanvas() {
	return canvas;
}

void gAppManager::setScreenSize(int width, int height) {
	canvas->setScreenSize(width, height);
}

void gAppManager::setFramerate(int targetFramerate) {
	desiredframerate = targetFramerate;
	desiredframetime = std::chrono::nanoseconds(int(1000000000 / desiredframerate));
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


int gAppManager::getFramerate() {
    return int(1000000000 / desiredframetime.count());
}

double gAppManager::getElapsedTime() {
	return desiredframetime.count();
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
	if (gRenderer::getScreenScaling() > gRenderer::SCREENSCALING_NONE) {
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
		if (gRenderer::getScreenScaling() > gRenderer::SCREENSCALING_NONE) {
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
		if (gRenderer::getScreenScaling() > gRenderer::SCREENSCALING_NONE) {
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
	for (upi = 0; upi < gBasePlugin::usedplugins.size(); upi++) gBasePlugin::usedplugins[upi]->mouseScrolled(xoffset, yoffset);
	canvasmanager->getCurrentCanvas()->mouseScrolled(xoffset, yoffset);
}
