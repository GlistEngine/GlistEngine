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

/**
 * This function can be used to create an empty gSpriteAnimation pointer.
 *
 * @return Dynamically created gSpriteAnimation's pointer.
 */
gSpriteAnimation* createSpriteAnimation();

/**
 * This function can be used to create a gSpriteAnimation with a pre-allocated
 * gImage list. So that way you have more control over the images, and don't have
 * to manually add frames into the animation via addFrame function.
 *
 * @param frameList Either the pointer to the first item of the list, or the list.
 * @param listSize The number of how many gImage's inside the list.
 *
 * @return Dynamically created gSpriteAnimation's pointer.
 */
gSpriteAnimation* createSpriteAnimation(gImage* frameList, int listSize);

/**
 * This function can be used to create a gSpriteAnimation with a pre-allocated
 * gImage list and a gAnimationTrigger. So that way you have more control over
 * the images, and don't have to manually add frames into the animation via
 * addFrame function.
 *
 * To create the gAnimationTrigger, simply use createTrigger function with the
 * required parameters.
 *
 * @see createTrigger()
 *
 * @param frameList Either the pointer to the first item of the list, or the list.
 * @param listSize The number of how many gImage's inside the list.
 *
 * @return Dynamically created gSpriteAnimation's pointer.
 */
gSpriteAnimation* createSpriteAnimation(gImage* frameList, int listSize, gAnimationTriggerBase* animationTrigger);

/**
 * This class can be used to ease the use and declarations of animations. Usually,
 * to create an animation, developers have to manually declare an array of gImage's
 * and load images into those. Then somewhere in the code, a logic has to be created
 * to animate those gImage's.
 *
 * Using this class alone is not recommended, but still can be used if you want to
 * manually arrange multiple animation's condition triggers. Instead, this class
 * should be used with a gSpriteAnimator, and should be added as a animation inside
 * that class.
 *
 * To create a gSpriteAnimation, the overloaded createSpriteAnimation functions can
 * be used for creating a gSpriteAnimation with different type of settings and
 * initializations.
 *
 * For now, the class only supports only one condition trigger, but multiple triggers
 * are coming soon.
 */
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

#define TRIGGER_CONDITION_LESS gSpriteAnimation::CONDITION_LESS
#define TRIGGER_CONDITION_GREATER gSpriteAnimation::CONDITION_GREATER
#define TRIGGER_CONDITION_EQUAL gSpriteAnimation::CONDITION_EQUAL
#define TRIGGER_CONDITION_FALSE gSpriteAnimation::CONDITION_FALSE
#define TRIGGER_CONDITION_TRUE gSpriteAnimation::CONDITION_TRUE

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

template<typename T>
class gAnimationTrigger : public gAnimationTriggerBase {
public:
	bool isConditionTriggered() {
		int value1 = *((int*)var1);
		int value2;
		if(var2 != nullptr) value2 = *((int*)var2);

		if(condition == gSpriteAnimation::CONDITION_LESS) {
			return value1 < value2;
		}
		else if (condition == gSpriteAnimation::CONDITION_GREATER) {
			return value1 > value2;
		}
		else if (condition == gSpriteAnimation::CONDITION_EQUAL) {
			return value1 == value2;
		}
		else if (condition == gSpriteAnimation::CONDITION_FALSE) {
			return !value1;
		}
		else if (condition == gSpriteAnimation::CONDITION_TRUE) {
			return value1;
		}
		return false;
	}
};

/**
 * This function creates a condition trigger that can be used with a gSpriteAnimation.
 *
 * @param condition A condition enumeration that is declared in gSpriteAnimation.
 * @param var1 Pointer to the first variable
 * @param var2 Pointer to the second variable (Optional)
 *
 * @return Pointer to the dynamically created gAnimationTrigger.
 */
template<typename T>
gAnimationTriggerBase* createTrigger(int condition, T* var1, T* var2 = nullptr) {
	gAnimationTriggerBase* trigger = new gAnimationTrigger<T>();

	trigger->addTrigger(condition, var1, var2);

	return trigger;
}

#endif /* ANIMATION_GANIMATION_H_ */
