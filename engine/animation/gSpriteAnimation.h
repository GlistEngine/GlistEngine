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
class gAnimationTriggerBase;
class gAnimationTriggerInt;
class gAnimationTriggerLong;
class gAnimationTriggerFloat;
class gAnimationTriggerDouble;
class gAnimationTriggerBool;

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
gSpriteAnimation* createSpriteAnimation();
gSpriteAnimation* createSpriteAnimation(gImage* frameList, int listSize);
gSpriteAnimation* createSpriteAnimation(gImage* frameList, int listSize, gAnimationTriggerBase* animationTrigger);

gAnimationTriggerBase* createIntTrigger(int condition, int* var1, int* var2);
gAnimationTriggerBase* createLongTrigger(int condition, long long* var1, long long* var2);
gAnimationTriggerBase* createFloatTrigger(int condition, float* var1, float* var2);
gAnimationTriggerBase* createDoubleTrigger(int condition, double* var1, double* var2);
gAnimationTriggerBase* createBoolTrigger(int condition, bool* var1);

class gAnimationTriggerBase {
public:
	virtual ~gAnimationTriggerBase();

	void addTrigger(int condition, void* var1, void* var2 = nullptr);
	virtual bool isConditionTriggered() = 0;
protected:
	int condition;
	void *var1;
	void *var2;
};

class gAnimationTriggerInt : public gAnimationTriggerBase {
public:
	bool isConditionTriggered();
};

class gAnimationTriggerLong : public gAnimationTriggerBase {
public:
	bool isConditionTriggered();
};

class gAnimationTriggerFloat : public gAnimationTriggerBase {
public:
	bool isConditionTriggered();
};

class gAnimationTriggerDouble : public gAnimationTriggerBase {
public:
	bool isConditionTriggered();
};

class gAnimationTriggerBool : public gAnimationTriggerBase {
public:
	bool isConditionTriggered();
};

class gSpriteAnimation : public gObject {
public:
	static const int CONDITION_LESS, CONDITION_GREATER, CONDITION_EQUAL, CONDITION_FALSE, CONDITION_TRUE;

	gSpriteAnimation();
	virtual ~gSpriteAnimation();

	void addFrame(gImage* frame);
	void loadFrame(const std::string& framePath);

	void setLoop(bool isLooped);
	void setFps(int fps);
	int getFps();

	void addConditionTrigger(gAnimationTriggerBase* trigger);
	bool isConditionTriggered();

	void reset();

	void update();
	void draw(int x, int y);
	void draw(int x, int y, int w, int h);

private:
	bool shouldStop();

	int fps;
	int framecount;
	int framecounter;
	int currentframe;

	bool islooped;

	gAnimationTriggerBase* trigger;
	bool hastriggered;

	std::vector<gImage*> frames;
};

#endif /* ANIMATION_GANIMATION_H_ */
