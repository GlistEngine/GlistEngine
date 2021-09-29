/*
 * gSpriteAnimator.cpp
 *
 *  Created on: 22 Eyl 2021
 *      Author: kayra
 */

#include "gSpriteAnimator.h"

gSpriteAnimator::gSpriteAnimator() {
	currentanimation = 0;
	autoassignedids = 0;
}

gSpriteAnimator::~gSpriteAnimator() {
	// TODO Auto-generated destructor stub
}

int gSpriteAnimator::addAnimation(gSpriteAnimation* animation) {
	animationids.push_back(autoassignedids++);
	animations.insert({autoassignedids, animation});

	return autoassignedids;
}

int gSpriteAnimator::addAnimation(gSpriteAnimation* animation, int animationId) {
	animationids.push_back(animationId);
	animations.insert({animationId, animation});

	return animationId;
}

void gSpriteAnimator::changeAnimation(int animationId) {
	currentanimation = animationId;
}

gSpriteAnimation* gSpriteAnimator::getAnimation(int animationId) {
	return animations[animationId];
}

void gSpriteAnimator::update() {
	for(int i = 0; i < animationids.size(); i++) {
		if(animations[animationids[i]]->isConditionTriggered()) {
			animations[currentanimation]->reset();
			currentanimation = animationids[i];
		}
	}
	animations[currentanimation]->update();
}

void gSpriteAnimator::draw(int x, int y) {
	animations[currentanimation]->draw(x, y);
}

void gSpriteAnimator::draw(int x, int y, float rotation) {
	animations[currentanimation]->draw(x, y, rotation);
}

void gSpriteAnimator::draw(int x, int y, int w, int h) {
	animations[currentanimation]->draw(x, y, w, h);
}

void gSpriteAnimator::draw(int x, int y, int w, int h, float rotation) {
	animations[currentanimation]->draw(x, y, w, h, rotation);
}
