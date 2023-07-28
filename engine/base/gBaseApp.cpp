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

gBaseApp::gBaseApp(int argc, char **argv) {
	gSeedRandom();
	this->argc = argc;
	this->argv = argv;
}

gBaseApp::~gBaseApp() {
}

void gBaseApp::setup() {

}

void gBaseApp::update() {

}

void gBaseApp::start() {

}

void gBaseApp::stop() {

}

void gBaseApp::setCurrentCanvas(gBaseCanvas* currentCanvas) {
	appmanager->setCurrentCanvas(currentCanvas);
}

gBaseCanvas* gBaseApp::getCurrentCanvas() {
	return appmanager->getCurrentCanvas();
}

void gBaseApp::setTargetFramerate(int targetFramerate) {
	appmanager->setTargetFramerate(targetFramerate);
}

int gBaseApp::getFramerate() {
	return appmanager->getFramerate();
}

int gBaseApp::getTargetFramerate() {
	return appmanager->getTargetFramerate();
}

double gBaseApp::getElapsedTime() {
	return appmanager->getElapsedTime();
}

void gBaseApp::enableVsync() {
	appmanager->enableVsync();
}

void gBaseApp::disableVsync() {
	appmanager->disableVsync();
}
