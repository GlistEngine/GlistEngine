/*
 * gGUIAppThread.cpp
 *
 *  Created on: Aug 9, 2024
 *      Author: Noyan Culum
 */

#include "gGUIAppThread.h"
#include "gCanvasManager.h"


gGUIAppThread::gGUIAppThread(gBaseApp* root) {
	this->root = root;
	setTargetFramerate(root->getTargetFramerate());
}

gGUIAppThread::~gGUIAppThread() {
}

void gGUIAppThread::threadFunction() {
	gCanvasManager* canvasmanager = root->getAppManager()->getCanvasManager();
    gBaseCanvas* canvas = nullptr;

    while(true) {
		root->update();

	    if(canvasmanager) canvas = canvasmanager->getCurrentCanvas();
	    if(canvas) canvas->update();

	    sleep(sleepdurationmillis);
	}
}

void gGUIAppThread::setTargetFramerate(int framerate) {
	targetframerate = framerate;
	sleepdurationmillis = std::chrono::duration<double, std::milli>{ 1000.0 / (double)targetframerate };
}

int gGUIAppThread::getTargetFramerate() {
	return targetframerate;
}
