/*
 * gSpriteAnimator.cpp
 *
 *  Created on: 22 Eyl 2021
 *      Author: kayra
 */

#include "gSpriteAnimator.h"

const int gSpriteAnimator::CONDITION_LESS = 0;
const int gSpriteAnimator::CONDITION_GREATER = 1;
const int gSpriteAnimator::CONDITION_EQUAL = 2;
const int gSpriteAnimator::CONDITION_FALSE = 3;
const int gSpriteAnimator::CONDITION_TRUE = 4;

gSpriteAnimator::gSpriteAnimator() {
	currentanimation = 0;
}

gSpriteAnimator::~gSpriteAnimator() {
	// TODO Auto-generated destructor stub
}

void gSpriteAnimator::addAnimation(int animationId, gSpriteAnimation* animation) {
	animations.insert({animationId, animation});
}

void gSpriteAnimator::changeAnimation(int animationId) {
	currentanimation = animationId;
}

void gSpriteAnimator::update() {
	animations[currentanimation]->update();
}

void gSpriteAnimator::draw(int x, int y) {
	animations[currentanimation]->draw(x, y);
}

void gSpriteAnimator::draw(int x, int y, int w, int h) {
	animations[currentanimation]->draw(x, y, w, h);
}
