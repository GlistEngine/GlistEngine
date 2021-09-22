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
	static const int CONDITION_LESS, CONDITION_GREATER, CONDITION_EQUAL, CONDITION_FALSE, CONDITION_TRUE;

	gSpriteAnimator();
	virtual ~gSpriteAnimator();

	void addAnimation(int animationId, gSpriteAnimation* animation);
	void changeAnimation(int animationId);

	void addAnimationTriggerInt(int animationId, int condition, int* var1, int* var2);
	void addAnimationTriggerFloat(int animationId, int condition, long long* var1, long long* var2);
	void addAnimationTriggerFloat(int animationId, int condition, float* var1, float* var2);
	void addAnimationTriggerDouble(int animationId, int condition, double* var1, double* var2);
	void addAnimationTriggerBool(int animationId, int condition, bool* var);

	void update();
	void draw(int x, int y);
	void draw(int x, int y, int w, int h);

private:
	int currentanimation;
	std::map<int, gSpriteAnimation*> animations;
};

#endif /* ANIMATION_GSPRITEANIMATOR_H_ */
