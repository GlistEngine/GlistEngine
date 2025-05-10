//
// Created by Metehan Gezer on 10/05/2025.
//

#ifndef GRAPHICS_GMODELANIMATOR_H
#define GRAPHICS_GMODELANIMATOR_H

#include "gModel.h"

/**
 * @class gModelAnimator
 * @brief Handles animation logic for a gModel instance, including switching, playback, and timing.
 *
 * Supports multiple animation modes and allows defining transitions between animations.
 */
class gModelAnimator {
public:
    gModelAnimator();
    ~gModelAnimator();

    /**
     * @enum Mode
     * @brief Defines playback behavior for animations.
     */
    enum Mode {
        MODE_HOLD_ON_LAST_FRAME,
        MODE_ONCE,
        MODE_REPEAT
    };

    /**
     * @enum TriggerType
     * @brief Defines triggers for transitioning animations.
     */
    enum TriggerType {
        TRIGGERTYPE_AFTER_COMPLETION,
    };

    /**
     * @brief Associates a gModel with this animator.
     * @param model The model to animate.
     */
    void setModel(gModel* model) {
        this->model = model;
    }

    /**
     * @brief Adds a new animation to the animator.
     * @param id Unique identifier for the animation.
     * @param startframe Starting frame of the animation.
     * @param endframe Ending frame of the animation. Cannot be the same as startframe
     * @param mode Animation playback mode.
     * @param isDefault Whether this animation is the default on bake.
     */
    void addAnimation(int id, int startframe, int endframe, float speed, Mode mode, bool isDefault = false);

    /**
     * @brief Adds a transition to another animation.
     * @param triggerType The trigger condition.
     * @param srcAnimation Source animation ID.
     * @param dstAnimation Destination animation ID.
     */
    void addTransition(TriggerType triggerType, int srcAnimation, int dstAnimation);

    /**
     * @brief Finalizes the animator setup. Should be called before use and after the animator is fully configured.
     */
    void bake();

    /**
     * @brief Sets a global animation speed multiplier.
     * @param speed Playback speed (1.0 = normal).
     */
    void setGeneralSpeed(float speed) {
        this->speed = speed;
    }

    /**
     * @brief Starts transitioning to a different animation.
     * @param animationId The animation to play.
     */
    void triggerAnimation(int animationId);

    void update();


private:
    struct TransitionData {
        int targetanimation; ///< Target animation to switch to.
    };
    struct AnimationData {
        int id;
        Mode mode;
        float speed;
        int startframe;
        int endframe;
        std::unordered_map<TriggerType, TransitionData> transitions;

    };

    gModel* model = nullptr;
    std::unordered_map<int, AnimationData> animations;
    int defaultanimation = 0;
    int maxframenum = 0;

    int currentanimation;
    int nextanimation;
    float nextframetime = 0;
    float speed = 1.0f;

private:
    void prepareAnimation(int animationId);
};


#endif //GRAPHICS_GMODELANIMATOR_H
