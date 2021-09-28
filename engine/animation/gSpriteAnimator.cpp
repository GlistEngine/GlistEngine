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

int gSpriteAnimator::addAnimation(gSpriteAnimation* animation, int animationId) {
	int animid;
	if(animationId == -1) animid = autoassignedids++;
	else animid = animationId;
	animationids.push_back(animid);
	animations.insert({animid, animation});

	return animid;
}

void gSpriteAnimator::changeAnimation(int animationId) {
	currentanimation = animationId;
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

void gSpriteAnimator::draw(int x, int y, int w, int h) {
	animations[currentanimation]->draw(x, y, w, h);
}
