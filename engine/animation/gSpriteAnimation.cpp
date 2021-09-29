/*
 * gAnimation.cpp
 *
 *  Created on: 22 Eyl 2021
 *      Author: kayra
 */

#include "gSpriteAnimation.h"

const int gSpriteAnimation::CONDITION_LESS = 0;
const int gSpriteAnimation::CONDITION_GREATER = 1;
const int gSpriteAnimation::CONDITION_EQUAL = 2;
const int gSpriteAnimation::CONDITION_FALSE = 3;
const int gSpriteAnimation::CONDITION_TRUE = 4;

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

gSpriteAnimation* createSpriteAnimation(gImage* frameList, int listSize, gAnimationTriggerBase* animationTrigger) {
	gSpriteAnimation* animation = createSpriteAnimation(frameList, listSize);

	animation->addConditionTrigger(animationTrigger);

	return animation;
}

gSpriteAnimation::gSpriteAnimation() {
	fps = 0;
	framecount = 0;
	framecounter = 0;
	currentframe = 0;
	islooped = true;
	hastriggered = false;
	issrcset = false;
	trigger = nullptr;
	isflippedh = false;
	isflippedv = false;
	srcwidth = 0;
	srcheight = 0;
}

gSpriteAnimation::~gSpriteAnimation() {
	for(int i = 0; i < frames.size(); i++) {
		delete frames[i];
	}
	frames.clear();
	delete trigger;
}

void gSpriteAnimation::addFrame(gImage* frame) {
	if(!issrcset) {
		issrcset = true;
		srcwidth = frame->getWidth();
		srcheight = frame->getHeight();
		srcrect.set(0, 0, srcwidth, srcheight);
	}

	frames.push_back(frame);
	framecount++;
}

void gSpriteAnimation::loadFrame(const std::string &framePath) {
	gImage* img = new gImage();
	img->loadImage(framePath);
	addFrame(img);
}

void gSpriteAnimation::setLoop(bool isLooped) {
	islooped = isLooped;
}

void gSpriteAnimation::setFps(int fps) {
	this->fps = fps;
}

int gSpriteAnimation::getFps() {
	return fps;
}

void gSpriteAnimation::reset() {
	framecounter = 0;
	currentframe = 0;
	hastriggered = false;
}

void gSpriteAnimation::addConditionTrigger(gAnimationTriggerBase* trigger) {
	this->trigger = trigger;
}

bool gSpriteAnimation::isConditionTriggered() {
	if(!trigger || hastriggered) return false;

	hastriggered = trigger->isConditionTriggered();

	return hastriggered;
}

void gSpriteAnimation::setFlipV(bool shouldFlip) {
	srcrect.set(0, 0, srcwidth, (shouldFlip ? -1 : 1) * srcheight);
}

void gSpriteAnimation::setFlipH(bool shouldFlip) {
	srcrect.set(0, 0, (shouldFlip ? -1 : 1) * srcwidth, srcheight);
}

bool gSpriteAnimation::isFlippedV() {
	return isflippedv;
}

bool gSpriteAnimation::isFlippedH() {
	return isflippedh;
}

void gSpriteAnimation::update() {

	if(shouldStop()) return;

	if(framecounter >= fps) {
		currentframe++;
		if(currentframe >= framecount) currentframe = 0;
		framecounter = 0;
		return;
	}
	framecounter++;
}

void gSpriteAnimation::draw(int x, int y) {
	draw(x, y, srcwidth, srcheight);
}

void gSpriteAnimation::draw(int x, int y, float rotation) {
	draw(x, y, srcwidth, srcheight, rotation);
}

void gSpriteAnimation::draw(int x, int y, int w, int h) {
	draw(x, y, w, h, 0.0f);
}

void gSpriteAnimation::draw(int x, int y, int w, int h, float rotation) {
	dstrect.set(x, y, x + w, y + h);
	draw(rotation);
}

void gSpriteAnimation::draw(float rotation) {
	frames[currentframe]->drawSub(srcrect, dstrect, rotation);
}

bool gSpriteAnimation::shouldStop() {
	return !islooped ? currentframe >= framecount - 1: false;
}

// --- TRIGGER FUNCTION DEFINITIONS --  \\

gAnimationTriggerBase::~gAnimationTriggerBase() {}

void gAnimationTriggerBase::addTrigger(int condition, void* var1, void* var2) {
	this->var1 = var1;
	this->var2 = var2;
	this->condition = condition;
}
