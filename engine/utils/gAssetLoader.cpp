/*
 * gAssetLoader.cpp
 *
 *  Created on: May 11, 2021
 *      Author: noyan
 */

#include "gAssetLoader.h"


gAssetLoader::gAssetLoader() {
    signal = SIGNAL_NONE;
    counter = 0;
    loadedassetnum = 0;
    transferfinished = false;
    isupdateneeded = false;
	start();
}

gAssetLoader::~gAssetLoader() {
}

void gAssetLoader::load(gImage& image, std::string fullPath, short signal) {
	assetToLoad atl(image, fullPath, signal);
	send(atl);
}

void gAssetLoader::loadImage(gImage& image, std::string imagePath, short signal) {
	load(image, gGetImagesDir() + imagePath, signal);
}


void gAssetLoader::threadFunction() {
	assetToLoad assettoload;
	while(receive(assettoload)) {
		switch(assettoload.type) {
		case TYPE_IMAGE:
			assettoload.image->loadData(assettoload.filename);
			sendToUpdate(assettoload);
			isupdateneeded = true;
			isrunning = false;
			isdone = true;
			break;
		default:
			break;
		}
        loadedassetnum++;
	}
}

void gAssetLoader::update() {
	if(!isupdateneeded) return;

	assetToLoad assettoupdate;
	if (receiveToUpdate(assettoupdate)) {
        switch(assettoupdate.type) {
        case TYPE_IMAGE:
        	assettoupdate.image->useData();
        	signal = assettoupdate.assetsignal;
        	break;
        default:
        	break;
        }
    	counter++;
		if(counter == loadedassetnum) isupdateneeded = false;
	}
}


int gAssetLoader::getLoadedAssetNum() {
    return loadedassetnum;
}

short gAssetLoader::getSignal() {
	return signal;
}

void gAssetLoader::resetSignal() {
	signal = SIGNAL_NONE;
}

int gAssetLoader::getCounter() {
	return counter;
}

void gAssetLoader::reset() {
	loadedassetnum = 0;
	counter = 0;
	signal = SIGNAL_NONE;
}

bool gAssetLoader::isUpdateNeeded() {
	return isupdateneeded;
}


