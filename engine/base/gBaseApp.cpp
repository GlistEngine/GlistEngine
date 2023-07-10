/*
 * gBaseApp.cpp
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

#include "gAppManager.h"
#include "gBaseApp.h"


gBaseApp::gBaseApp() {
	gSeedRandom();
	argc = 0;
	argv = nullptr;
}

#ifndef ANDROID
gBaseApp::gBaseApp(int argc, char **argv) {
	appmanager = nullptr;
	gSeedRandom();
	this->argc = argc;
	this->argv = argv;
}
#endif

gBaseApp::~gBaseApp() {
}

void gBaseApp::setup() {

}

void gBaseApp::update() {

}

#ifdef ANDROID
void gBaseApp::pause() {

}

void gBaseApp::resume() {

}

void gBaseApp::start() {

}

void gBaseApp::stop() {

}
#endif

void gBaseApp::setCurrentCanvas(gBaseCanvas* currentCanvas) {
	gAppManager::get()->setCurrentCanvas(currentCanvas);
}

gBaseCanvas* gBaseApp::getCurrentCanvas() {
	return gAppManager::get()->getCurrentCanvas();
}

void gBaseApp::setTargetFramerate(int targetFramerate) {
	gAppManager::get()->setTargetFramerate(targetFramerate);
}

int gBaseApp::getFramerate() {
	return gAppManager::get()->getFramerate();
}

int gBaseApp::getTargetFramerate() {
	return gAppManager::get()->getTargetFramerate();
}

double gBaseApp::getElapsedTime() {
	return gAppManager::get()->getElapsedTime();
}

void gBaseApp::enableVsync() {
    gAppManager::get()->enableVsync();
}

void gBaseApp::disableVsync() {
    gAppManager::get()->disableVsync();
}
