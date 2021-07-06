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
	tempbasecanvas = nullptr;
	ismouseentered = false;
	buttonpressed[0] = false;
	buttonpressed[1] = false;
	buttonpressed[2] = false;
	pressed = 0;
	framerate = 60;
	millisecondsperframe = 1000 / framerate;
	minWorkTime = std::chrono::duration<double, std::milli>(millisecondsperframe);
	starttime = std::chrono::high_resolution_clock::now();
	timediff = std::chrono::high_resolution_clock::now() - starttime;
	timediff2 = std::chrono::high_resolution_clock::now() - starttime;
	delaycoef = 0.34f;
	delay = std::chrono::duration<double, std::milli>(delaycoef * 60 / framerate);
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

	tempbasecanvas = new gBaseCanvas(app);
	tempbasecanvas->setScreenSize(width, height);
	tempbasecanvas->setUnitScreenSize(unitWidth, unitHeight);
	tempbasecanvas->setScreenScaling(screenScaling);

	canvasmanager = new gCanvasManager();

	// Run app
	app->setup();
//	if (canvasmanager->getTempCanvas() != nullptr) {
//		canvasmanager->getTempCanvas()->setup(); // Commented out because was invoking first canvas's setup 2 times in the app launch
//	}

	starttime = std::chrono::high_resolution_clock::now();

	// Main loop
	while(!window->getShouldClose()) {
		canvasmanager->update();
		app->update();
		for (upi = 0; upi < gBasePlugin::usedplugins.size(); upi++) gBasePlugin::usedplugins[upi]->update();
		canvas = canvasmanager->getCurrentCanvas();
		if (canvas != nullptr) {
			canvas->update();
			for (upj = 0; upj < renderpassnum; upj++) {
				renderpassno = upj;
				canvas->clearBackground();
				canvas->draw();
			}
		}
		window->update();

		// Framerate adjustment
		timediff = std::chrono::high_resolution_clock::now() - starttime;
		if (timediff < minWorkTime) {
			std::this_thread::sleep_for(minWorkTime - timediff);
		}
		timediff2 = std::chrono::high_resolution_clock::now() - starttime;
		starttime = std::chrono::high_resolution_clock::now();
	}

	window->close();
}

void gAppManager::setWindow(gBaseWindow *baseWindow) {
	window = baseWindow;
}

gCanvasManager *gAppManager::getCanvasManager() {
	return canvasmanager;
}

void gAppManager::setCurrentCanvas(gBaseCanvas *baseCanvas) {
	canvas = baseCanvas;
	canvasmanager->setCurrentCanvas(canvas);
}

gBaseCanvas* gAppManager::getCurrentCanvas() {
	return canvas;
}

void gAppManager::setScreenSize(int width, int height) {
	tempbasecanvas->setScreenSize(width, height);
}

void gAppManager::setFramerate(int targetFramerate) {
	framerate = targetFramerate;
	millisecondsperframe = 1000 / framerate;
	minWorkTime = std::chrono::duration<double, std::milli>(millisecondsperframe);
	delay = std::chrono::duration<double, std::milli>(delaycoef * 60 / framerate);
}

std::string gAppManager::getAppName() {
	return appname;
}

int gAppManager::getFramerate() {
    return (int)(1000 / timediff2.count());
}

double gAppManager::getElapsedTime() {
	return timediff2.count();
}

void gAppManager::onCharEvent(unsigned int key) {
#if defined(WIN32) || defined(LINUX) || defined(APPLE)
	for (upi = 0; upi < gBasePlugin::usedplugins.size(); upi++) gBasePlugin::usedplugins[upi]->charPressed(key);
	canvasmanager->getCurrentCanvas()->charPressed(key);
#endif
}

void gAppManager::onKeyEvent(int key, int action) {
#if defined(WIN32) || defined(LINUX) || defined(APPLE)
	switch(action) {
	case GLFW_RELEASE:
		for (upi = 0; upi < gBasePlugin::usedplugins.size(); upi++) gBasePlugin::usedplugins[upi]->keyReleased(key);
		canvasmanager->getCurrentCanvas()->keyReleased(key);
		break;
	case GLFW_PRESS:
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
		for (upi = 0; upi < gBasePlugin::usedplugins.size(); upi++) gBasePlugin::usedplugins[upi]->mouseDragged(xpos, ypos, pressed);
		canvasmanager->getCurrentCanvas()->mouseDragged(xpos, ypos, pressed);
	} else {
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
