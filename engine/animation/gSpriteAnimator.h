/*
 * gSpriteAnimator.h
 *
 *  Created on: 22 Eyl 2021
 *      Author: kayra
 */

#ifndef ANIMATION_GSPRITEANIMATOR_H_
#define ANIMATION_GSPRITEANIMATOR_H_

#include "gSpriteAnimation.h"
#include <map>

class gSpriteAnimator : public gObject{
public:
	gSpriteAnimator();
	virtual ~gSpriteAnimator();

	void addAnimation(int animationId, gSpriteAnimation* animation);
	void changeAnimation(int animationId);

	void update();
	void draw(int x, int y);
	void draw(int x, int y, int w, int h);

private:
	int currentanimation;
	std::vector<int> animationids;
	std::map<int, gSpriteAnimation*> animations;
};

#endif /* ANIMATION_GSPRITEANIMATOR_H_ */
