/*
 * gAnimation.h
 *
 *  Created on: 22 Eyl 2021
 *      Author: kayra
 */

#ifndef ANIMATION_GANIMATION_H_
#define ANIMATION_GANIMATION_H_

#include <functional>
#include <memory>

#include "gImage.h"

enum TriggerCondition {
	LESS,
	GREATER,
	LESS_EQUAL,
	GREATER_EQUAL,
	EQUAL,
	FALSE,
	TRUE,

	MAX_TRIGGER_CONDITIONS,
};

/**
 * An individual instance of this class should not be created. This class is
 * only meant to be used as a parameter to createSpriteAnim() function. For
 * creating an instance, refer to the createTrigger function.
 *
 * @see createTrigger()
 */
class gAnimationTrigger {
public:
	template<typename First, typename Second>
	constexpr gAnimationTrigger(First* first, Second* second, TriggerCondition cond)
	{
		switch(cond)
		{
		case TriggerCondition::LESS:
			triggerfunc = [first, second]() -> bool { return *first < *second; };
			break;
		case TriggerCondition::GREATER:
			triggerfunc = [first, second]() -> bool { return *first > *second; };
			break;
		case TriggerCondition::LESS_EQUAL:
			triggerfunc = [first, second]() -> bool { return *first <= *second; };
			break;
		case TriggerCondition::GREATER_EQUAL:
			triggerfunc = [first, second]() -> bool { return *first >= *second; };
			break;
		case TriggerCondition::EQUAL:
			triggerfunc = [first, second]() -> bool { return *first == *second; };
			break;
		case TriggerCondition::FALSE:
			triggerfunc = [first]() -> bool { return *first == false; };
			break;
		case TriggerCondition::TRUE:
			triggerfunc = [first]() -> bool { return *first == true; };
			break;
		}
	}

	bool isConditionTriggered() {
		return triggerfunc();
	}
private:
	std::function<bool()> triggerfunc;
};

class gSpriteAnimation : public gObject {
public:
	gSpriteAnimation();
	virtual ~gSpriteAnimation();

	/**
	 * Loads an image object from the assets/image folder.
	 *
	 * @param framePath The relative path to image file from assets/images
	 */
	void loadFrame(const std::string& framePath);

	void setLoop(bool isLooped);
	void setFps(int fps);
	int  getFps();

	void addConditionTrigger(std::shared_ptr<gAnimationTrigger> trigger);
	bool isConditionTriggered();

	/**
	 * Resets the animation's framecounter and current frame.
	 */
	void reset();

	/**
	 * Updates the animation's animation state.
	 *
	 * Should be called from Canvas's update function if created seperately.
	 */
	void update();

	/**
	 * Draws the animation's current sprite to the specified x and y
	 * coordinates.
	 *
	 * @param x The x coordinate to the draw the current sprite in the anim
	 * @param y The y coordinate to the draw the current sprite in the anim
	 */
	void draw(int x, int y);

	/**
	 * Draws the animation's current sprite to the specified x and y
	 * coordinates and scales it by w and h in the x and y axis.
	 *
	 * @param x The x coordinate to the draw the current sprite in the anim
	 * @param y The y coordinate to the draw the current sprite in the anim
	 * @param w The scaling on the x axis.
	 * @param h The scaling on the h axis.
	 */
	void draw(int x, int y, int w, int h);

private:
	bool shouldStop();

	int fps;
	int framecount;
	int framecounter;
	int currentframe;

	bool islooped;

	std::shared_ptr<gAnimationTrigger> trigger;
	bool hastriggered;

	std::vector<std::unique_ptr<gImage>> frames;
};

/**
 * This function can be used to create a gSpriteAnimation with a specified
 * animation trigger. The return of this should be kept, then sprites should
 * be added with loadFrame function. Then the pointer should be passed into
 * the game object's gSpriteAnimator via addAnimation function.
 *
 * ============================================================================
 * Example:
 * (ator is an instance of a gSpriteAnimator)
 *
 * gSpriteAnimation* anim1 = createSpriteAnim(TRIGGER_LESS(&a, &b));
 * anim1->loadFrame("characters/mario_walk_1.png");
 * anim1->loadFrame("characters/mario_walk_2.png");
 * ...
 * anim1->setFps(3);
 *
 * gSpriteAnimation* anim2 = createSpriteAnim(TRIGGER_GREATER(&b, &b));
 * anim2->loadFrame("characters/mario_run_1.png");
 * anim2->loadFrame("characters/mario_run_2.png");
 * ...
 * anim2->setFps(1);
 *
 * ator.addAnimation(0, anim1);
 * ator.addAnimation(1, anim2);
 * ============================================================================
 *
 * To create the gAnimationTrigger, simply use createTrigger function with the
 * required parameters.
 *
 * @see createTrigger
 *
 * @param animationTrigger The shared_ptr reference to the gAnimationTrigger
 * that was created by createTrigger
 *
 * @return Dynamically created gSpriteAnimation's pointer.
 */
gSpriteAnimation* createSpriteAnim(std::shared_ptr<gAnimationTrigger> animationTrigger);

/**
 * When creating a new gAnimationTrigger object, this function MUST be used to
 * create it. And this function should always be used with createSpriteAnim.
 *
 * Otherwise it creates an ownership problem.
 *
 * @return The gAnimationTrigger object that has been created.
 */
template<typename First, typename Second>
std::shared_ptr<gAnimationTrigger> createTrigger(First* first, Second* second, TriggerCondition cond) {
	return std::shared_ptr<gAnimationTrigger>(new gAnimationTrigger{first, second, cond});
}

// These macros can be used instead of calling the createTrigger function.
// The macro parameters First and Second should be the pointer to the values
// that are to be compared.
#define TRIGGER_NONE                         nullptr
#define TRIGGER_LESS(First, Second)          createTrigger(First, Second, TriggerCondition::LESS)
#define TRIGGER_GREATER(First, Second)       createTrigger(First, Second, TriggerCondition::GREATER)
#define TRIGGER_LESS_EQUAL(First, Second)    createTrigger(First, Second, TriggerCondition::LESS_EQUAL)
#define TRIGGER_GREATER_EQUAL(First, Second) createTrigger(First, Second, TriggerCondition::GREATER_EQUAL)
#define TRIGGER_EQUAL(First, Second)         createTrigger(First, Second, TriggerCondition::EQUAL)
#define TRIGGER_FALSE(First, Second)         createTrigger(First, Second, TriggerCondition::FALSE)
#define TRIGGER_TRUE(First, Second)          createTrigger(First, Second, TriggerCondition::TRUE)

#endif /* ANIMATION_GANIMATION_H_ */
