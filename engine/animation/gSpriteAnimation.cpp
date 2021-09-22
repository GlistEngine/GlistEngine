/*
 * gAnimation.cpp
 *
 *  Created on: 22 Eyl 2021
 *      Author: kayra
 */

#include "gSpriteAnimation.h"

gSpriteAnimation* createSpriteAnimation() {
	return new gSpriteAnimation();
}

gSpriteAnimation* createSpriteAnimation(gImage* frameList, int listSize) {
	gSpriteAnimation* animation = createSpriteAnimation();

	for(int i = 0; i < listSize; i++) {
		animation->addFrame(&frameList[i]);
	}

	return animation;
}

gSpriteAnimation::gSpriteAnimation() {
	fps = 0;
	framecount = 0;
	framecounter = 0;
	currentframe = 0;

	condition = 0;
}

gSpriteAnimation::~gSpriteAnimation() {
	for(int i = 0; i < frames.size(); i++) {
		delete frames[i];
	}
	frames.clear();
}

void gSpriteAnimation::addFrame(gImage* frame) {
	frames.push_back(frame);
	framecount++;
}

void gSpriteAnimation::loadFrame(const std::string &framePath) {
	gImage* img = new gImage();
	img->loadImage(framePath);
	addFrame(img);
}

void gSpriteAnimation::setFps(int fps) {
	this->fps = fps;
}

int gSpriteAnimation::getFps() {
	return fps;
}

void gSpriteAnimation::update() {
	if(framecounter >= fps) {
		currentframe++;
		if(currentframe >= framecount) currentframe = 0;
		framecounter = 0;
		return;
	}
	framecounter++;
}

void gSpriteAnimation::draw(int x, int y) {
	frames[currentframe]->draw(x, y);
}

void gSpriteAnimation::draw(int x, int y, int w, int h) {
	frames[currentframe]->draw(x, y, w, h);
}
