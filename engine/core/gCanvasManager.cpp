/*
 * ofDisplayManager.cpp
 *
 *  Created on: May 1, 2014
 *      Author: noyan
 */

#include "gCanvasManager.h"

#include "gTracy.h"

gCanvasManager::gCanvasManager() {
	currentCanvas = nullptr;
	tempCanvas = nullptr;
	displaychangement = DISPLAY_CHANGE_NONE;
}

gCanvasManager::~gCanvasManager() {

}

void gCanvasManager::update() {
	G_PROFILE_ZONE_SCOPED_N("gCanvasManager::update");
	switch(displaychangement) {
		case DISPLAY_CHANGE_NONE:
			break;
		case DISPLAY_CHANGE_CURRENT:
			if (currentCanvas != nullptr) {
				currentCanvas->hideNotify();
				delete currentCanvas;
			}
			if (tempCanvas != nullptr) {
				currentCanvas = tempCanvas;
				currentCanvas->showNotify();
				currentCanvas->setup();
				displaychangement = DISPLAY_CHANGE_NONE;
				tempCanvas = nullptr;
			}
			break;
		default:
			break;
	}
}

void gCanvasManager::setCurrentCanvas(gBaseCanvas *ocnv) {
	tempCanvas = ocnv;
	displaychangement = DISPLAY_CHANGE_CURRENT;
}

gBaseCanvas* gCanvasManager::getCurrentCanvas() {
	return currentCanvas;
}

gBaseCanvas* gCanvasManager::getTempCanvas() {
	return tempCanvas;
}



