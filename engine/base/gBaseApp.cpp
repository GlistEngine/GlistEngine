/*
 * gBaseApp.cpp
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

#include <gEngine.h>
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
#endif

void gBaseApp::setCurrentCanvas(gBaseCanvas* currentCanvas) {
	gEngine::get()->setCurrentCanvas(currentCanvas);
}

gBaseCanvas* gBaseApp::getCurrentCanvas() {
	return gEngine::get()->getCurrentCanvas();
}

void gBaseApp::setTargetFramerate(int targetFramerate) {
	gEngine::get()->setTargetFramerate(targetFramerate);
}

int gBaseApp::getFramerate() {
	return gEngine::get()->getFramerate();
}

int gBaseApp::getTargetFramerate() {
	return gEngine::get()->getTargetFramerate();
}

double gBaseApp::getElapsedTime() {
	return gEngine::get()->getElapsedTime();
}

void gBaseApp::enableVsync() {
    gEngine::get()->enableVsync();
}

void gBaseApp::disableVsync() {
    gEngine::get()->disableVsync();
}
