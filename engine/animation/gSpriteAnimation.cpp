/*
 * gAnimation.cpp
 *
 *  Created on: 22 Eyl 2021
 *      Author: kayra
 */

#include "gSpriteAnimation.h"

gSpriteAnimation* createSpriteAnim(std::shared_ptr<gAnimationTrigger> animationTrigger) {
	gSpriteAnimation* animation = new gSpriteAnimation{};

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

	trigger = nullptr;
}

gSpriteAnimation::~gSpriteAnimation() {
}

void gSpriteAnimation::loadFrame(const std::string &framePath) {
	auto img = std::make_unique<gImage>();
	img->loadImage(framePath);
	frames.push_back(std::move(img));
	framecount++;
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

void gSpriteAnimation::addConditionTrigger(std::shared_ptr<gAnimationTrigger> trigger) {
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
	logi("update");
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
