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
#include <memory>

/**
 * An instance of this class should be created for each object that is to be
 * animated inside the game. For example; if the developer has a character,
 * an enemy and a door, three instances of this class should be created. Then,
 * the object's individual animations should be added to this class by calling
 * addAnimation.
 *
 * @see gSpriteAnimator::addAnimation(int, gSpriteAnimation*)
 * @see createSpriteAnim()
 */
class gSpriteAnimator : public gObject {
public:
	gSpriteAnimator();
	virtual ~gSpriteAnimator();

	/**
	 * The first argument is just an arbitrary number that is used assigned
	 * to the animation passed as the second argument. It can be used with
	 * the changeAnimation function to change into playing another animation.
	 *
	 * Second argument should always be created with createSpriteAnim function.
	 *
	 * @param animationId The id that will be given to the animation that's
	 * being passed in.
	 * @param animation The animation object
	 */
	void addAnimation(int animationId, gSpriteAnimation* animation);

	/**
	 * Changes the current playing animation to the specified animation. The
	 * animationId should be the id that was assigned to the gSpriteAnimation
	 * object passed in when calling addAnimation.
	 *
	 * @param animationId The animation id that was assigned to the
	 * gSpriteAnimation object.
	 */
	void changeAnimation(int animationId);

	/**
	 * Sets the fps of the given animation.
	 *
	 * @param animationId The animation id that was assigned to the
	 * gSpriteAnimation object.
	 */
	void setFps(int animationId, int fps);

	/**
	 * Call this update inside the related Canvas's update function.
	 */
	void update();

	/**
	 * The two parameter draw function.
	 *
	 * Call this function inside the draw function of the related Canvas. This
	 * draws the current animation on to the x and y coordinates of the screen.
	 *
	 * @param x The x coordinate for the sprite.
	 * @param y The y coordinate for the sprite.
	 */
	void draw(int x, int y);

	/**
	 * The two parameter draw function.
	 *
	 * Call this function inside the draw function of the related Canvas. This
	 * draws the current animation on to the x and y coordinates of the screen
	 * and scales it by w and h.
	 *
	 * @param x The x coordinate for the sprite.
	 * @param y The y coordinate for the sprite.
	 * @param w The scaling on the x axis.
	 * @param h The scaling on the h axis.
	 */
	void draw(int x, int y, int w, int h);

private:
	int currentanimation;
	std::map<int, std::unique_ptr<gSpriteAnimation>> animations;
};

#endif /* ANIMATION_GSPRITEANIMATOR_H_ */
