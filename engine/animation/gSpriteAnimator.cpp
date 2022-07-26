/*
 * gSpriteAnimator.cpp
 *
 *  Created on: 22 Eyl 2021
 *      Author: kayra
 */

#include "gSpriteAnimator.h"

gSpriteAnimator::gSpriteAnimator() {
	currentanimation = 0;
}

gSpriteAnimator::~gSpriteAnimator() {
	// TODO Auto-generated destructor stub
}

void gSpriteAnimator::addAnimation(int animationId, gSpriteAnimation* animation) {
	animations.insert(std::make_pair(animationId, std::move(std::unique_ptr<gSpriteAnimation>{animation})));
}

void gSpriteAnimator::changeAnimation(int animationId) {
	currentanimation = animationId;
}

void gSpriteAnimator::setFps(int animationId, int fps) {
	animations[animationId]->setFps(fps);
}

void gSpriteAnimator::update() {
	for(const auto& kv : animations) {

		if(kv.second->isConditionTriggered()) {
			animations[currentanimation]->reset();
			currentanimation = kv.first;
		}
	}
	animations[currentanimation]->update();
}

void gSpriteAnimator::draw(int x, int y) {
	animations[currentanimation]->draw(x, y);
}

void gSpriteAnimator::draw(int x, int y, int w, int h) {
	animations[currentanimation]->draw(x, y, w, h);
}
