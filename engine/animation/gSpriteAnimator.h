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

/**
 * This class is eases the use of animations and their state controls. A single
 * gSpriteAnimator is enough for multiple gSpriteAnimations of the same object
 *
 * To add an animation to the gSpriteAnimator, simply call addAnimation with
 * the needed parameters.
 *
 * Then for the first parameter, a manually created gSpriteAnimation pointer can
 * be passed, but for the ease of use, simply call createSpriteAnimation with the
 * needed parameters.
 *
 * Then for the animationId, developers can pass in their own
 * declared animationId's, if more control and the use of changeAnimation is
 * wanted.
 *
 * If nothing is passed, the animationId's will be auto assigned in an
 * ordered fashion and the id that's been assigned to that animation is returned.
 * For example the animationId of the first added animation will be 0, the
 * second will be 1 and so on.
 *
 * After initializations, gSpriteAnimator should be updated with the update method
 * inside the current canvas's update function. To draw, simply call the draw
 * function with needed parameters.
 */
class gSpriteAnimator : public gObject{
public:
	gSpriteAnimator();
	virtual ~gSpriteAnimator();

	int addAnimation(gSpriteAnimation* animation, int animationId = -1);
	void changeAnimation(int animationId);

	void update();
	void draw(int x, int y);
	void draw(int x, int y, int w, int h);

private:
	int currentanimation;
	std::vector<int> animationids;
	std::map<int, gSpriteAnimation*> animations;

	int autoassignedids;
};

#endif /* ANIMATION_GSPRITEANIMATOR_H_ */
