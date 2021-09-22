/*
 * gAnimation.h
 *
 *  Created on: 22 Eyl 2021
 *      Author: kayra
 */

#ifndef ANIMATION_GANIMATION_H_
#define ANIMATION_GANIMATION_H_

#include "gImage.h"

class gSpriteAnimation;


gSpriteAnimation* createSpriteAnimation();
/*
 * After calling this function, users should use "delete" to manually delete
 * the dynamically created gSpriteAnimation to avoid memory leaks or excessive
 * amounts of memory usage.
 *
 * This function should be used when there is already an allocated gImage array.
 *
 * @param frameList The pointer to the first member of the gImage array/pointer;
 * @param listSize The size of the image array;
 */
gSpriteAnimation* createSpriteAnimation(gImage* frameList, int listSize);

class gSpriteAnimation : public gObject {
public:
	gSpriteAnimation();
	virtual ~gSpriteAnimation();

	void addFrame(gImage* frame);
	void loadFrame(const std::string& framePath);

	void setFps(int fps);
	int getFps();

	void update();

	void draw(int x, int y);
	void draw(int x, int y, int w, int h);

private:
	int condition;

	int fps;
	int framecount;
	int framecounter;
	int currentframe;

	std::vector<gImage*> frames;
};

#endif /* ANIMATION_GANIMATION_H_ */
