/*
 * gBaseApp.cpp
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

#include "gBaseApp.h"



gBaseApp::gBaseApp() {
	appmanager = nullptr;
	gSeedRandom();
	argc = 0;
	argv = nullptr;
}

gBaseApp::gBaseApp(int argc, char **argv) {
	appmanager = nullptr;
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

void gBaseApp::setAppManager(gAppManager *appManager) {
	appmanager = appManager;
}

gAppManager* gBaseApp::getAppManager() {
	return appmanager;
}

void gBaseApp::setCurrentCanvas(gBaseCanvas* currentCanvas) {
	appmanager->setCurrentCanvas(currentCanvas);
}

gBaseCanvas* gBaseApp::getCurrentCanvas() {
	return appmanager->getCurrentCanvas();
}

void gBaseApp::setFramerate(int targetFramerate) {
	appmanager->setFramerate(targetFramerate);
}

int gBaseApp::getFramerate() {
	return appmanager->getFramerate();
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
