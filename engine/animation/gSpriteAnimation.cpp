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

gAnimationTriggerBase* createIntTrigger(int condition, int* var1, int* var2) {
	gAnimationTriggerBase* trigger = new gAnimationTriggerInt();

	trigger->addTrigger(condition, var1, var2);

	return trigger;
}

gAnimationTriggerBase* createLongTrigger(int condition, long long* var1, long long* var2) {
	gAnimationTriggerBase* trigger = new gAnimationTriggerLong();

	trigger->addTrigger(condition, var1, var2);

	return trigger;
}

gAnimationTriggerBase* createFloatTrigger(int condition, float* var1, float* var2) {
	gAnimationTriggerBase* trigger = new gAnimationTriggerFloat();

	trigger->addTrigger(condition, var1, var2);

	return trigger;
}

gAnimationTriggerBase* createDoubleTrigger(int condition, double* var1, double* var2) {
	gAnimationTriggerBase* trigger = new gAnimationTriggerDouble();

	trigger->addTrigger(condition, var1, var2);

	return trigger;
}

gAnimationTriggerBase* createBoolTrigger(int condition, bool* var1) {
	gAnimationTriggerBase* trigger = new gAnimationTriggerBool();

	trigger->addTrigger(condition, var1);

	return trigger;
}

gSpriteAnimation::gSpriteAnimation() {
	fps = 0;
	framecount = 0;
	framecounter = 0;
	currentframe = 0;
	islooped = true;
	hastriggered = false;

	trigger = nullptr;
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
	frames[currentframe]->draw(x, y);
}

void gSpriteAnimation::draw(int x, int y, int w, int h) {
	frames[currentframe]->draw(x, y, w, h);
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

bool gAnimationTriggerInt::isConditionTriggered() {
	int value1 = *((int*)var1);
	int value2 = *((int*)var2);

	if(condition == gSpriteAnimation::CONDITION_LESS) {
		return value1 < value2;
	}
	else if (condition == gSpriteAnimation::CONDITION_GREATER) {
		return value1 > value2;
	}
	else if (condition == gSpriteAnimation::CONDITION_EQUAL) {
		return value1 == value2;
	}
	return false;
}

bool gAnimationTriggerLong::isConditionTriggered() {
	long value1 = *((long*)var1);
	long value2 = *((long*)var2);
	if(condition == gSpriteAnimation::CONDITION_LESS) {
		return value1 < value2;
	}
	else if (condition == gSpriteAnimation::CONDITION_GREATER) {
		return value1 > value2;
	}
	else if (condition == gSpriteAnimation::CONDITION_EQUAL) {
		return value1 == value2;
	}
	return false;
}

bool gAnimationTriggerFloat::isConditionTriggered() {
	float value1 = *((float*)var1);
	float value2 = *((float*)var2);
	if(condition == gSpriteAnimation::CONDITION_LESS) {
		return value1 < value2;
	}
	else if (condition == gSpriteAnimation::CONDITION_GREATER) {
		return value1 > value2;
	}
	else if (condition == gSpriteAnimation::CONDITION_EQUAL) {
		return value1 == value2;
	}
	return false;
}

bool gAnimationTriggerDouble::isConditionTriggered() {
	double value1 = *((double*)var1);
	double value2 = *((double*)var2);
	if(condition == gSpriteAnimation::CONDITION_LESS) {
		return value1 < value2;
	}
	else if (condition == gSpriteAnimation::CONDITION_GREATER) {
		return value1 > value2;
	}
	else if (condition == gSpriteAnimation::CONDITION_EQUAL) {
		return value1 == value2;
	}
	return false;
}

bool gAnimationTriggerBool::isConditionTriggered() {
	bool value1 = *((bool*)var1);
	return condition == gSpriteAnimation::CONDITION_TRUE ? value1 : !value1;
}
