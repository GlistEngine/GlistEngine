//
// Created by Metehan Gezer on 10/05/2025.
//

#include "gModelAnimator.h"
#include "gAppManager.h"
#include <cmath>

gModelAnimator::gModelAnimator() {

}

gModelAnimator::~gModelAnimator() {

}

void gModelAnimator::addAnimation(int id, int startframe, int endframe, float speed, Mode mode, bool isDefault) {
    animations.emplace(id,AnimationData{
            .id = id,
            .mode = mode,
            .speed = speed,
            .startframe = startframe,
            .endframe = endframe,
    });
    if (isDefault) {
        defaultanimation = id;
    }
    // Find the frame count using the highest frame number
    maxframenum = std::max(maxframenum, endframe + 1);
}

void gModelAnimator::addTransition(TriggerType triggerType, int srcAnimation, int dstAnimation) {
    animations[srcAnimation].transitions.emplace(
            triggerType,
            TransitionData{
                    .targetanimation = dstAnimation
            }
    );
}

void gModelAnimator::bake() {
    if (animations.empty() || model == nullptr) {
        return;
    }
    currentanimation = nextanimation = defaultanimation;
    model->setAnimationFrameNum(maxframenum);
    prepareAnimation(currentanimation);
}

void gModelAnimator::triggerAnimation(int animationId) {
    nextanimation = animationId;
}

void gModelAnimator::prepareAnimation(int animationId) {
    auto it = animations.find(animationId);
    if (it == animations.end()) return;
    AnimationData& data = it->second;
    currentanimationposition = 0.0f;
    model->animate(0.0f);
}

void gModelAnimator::update() {
    if (animations.empty() || !model || speed <= 0.0f) {
        return;
    }
    if (nextanimation != currentanimation) {
        currentanimation = nextanimation;
        prepareAnimation(currentanimation);
        currentanimationposition = 0.0f;
    }

    auto it = animations.find(currentanimation);
    if (it == animations.end()) {
        return;
    }
    AnimationData& data = it->second;
    float animationspeed = data.speed;
    float delta = appmanager->getElapsedTime();

    int totalFrames = data.endframe - data.startframe + 1;
    if (totalFrames <= 0) totalFrames = 1;
    float animationDuration = totalFrames / (60.0f * speed * animationspeed);

    float positionAdvance = delta / animationDuration;
    currentanimationposition += positionAdvance;

    if (currentanimationposition >= 1.0f) {
        switch (data.mode) {
            case MODE_HOLD_ON_LAST_FRAME:
                currentanimationposition = 1.0f;
                break;
            case MODE_ONCE: {
                currentanimationposition = 1.0f;
                auto it = data.transitions.find(TRIGGERTYPE_AFTER_COMPLETION);
                if (it != data.transitions.end()) {
                    nextanimation = it->second.targetanimation;
                }
                break;
            }
            case MODE_REPEAT:
                currentanimationposition = std::fmod(currentanimationposition, 1.0f);
                break;
        }
    }

    model->animate(currentanimationposition);
}