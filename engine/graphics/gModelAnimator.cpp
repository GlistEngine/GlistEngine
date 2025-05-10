//
// Created by Metehan Gezer on 10/05/2025.
//

#include "gModelAnimator.h"
#include "gAppManager.h"

gModelAnimator::gModelAnimator() {

}

gModelAnimator::~gModelAnimator() {

}

void gModelAnimator::addAnimation(int id, int startframe, int endframe, Mode mode, bool isDefault) {
    animations.emplace(id,AnimationData{
            .id = id,
            .mode = mode,
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
    model->setAnimationFrameNo(data.startframe);
}

void gModelAnimator::update() {
    if (animations.empty() || !model || speed <= 0.0f) {
        return;
    }

    float frameDuration = 1.0f / (60.0f * speed);
    float delta = appmanager->getElapsedTime(); // time since last frame
    nextframetime += delta;
    int frameAdvance = static_cast<int>(nextframetime / frameDuration);
    if (frameAdvance <= 0) {
        return;
    }
    nextframetime -= frameAdvance * frameDuration;
    if (nextanimation != currentanimation) {
        currentanimation = nextanimation;
        prepareAnimation(currentanimation);
    }
    auto it = animations.find(currentanimation);
    if (it == animations.end()) {
        return;
    }
    AnimationData& data = it->second;
    int newframe = model->getAnimationFrameNo() + frameAdvance;
    if (newframe >= data.endframe) {
        switch (data.mode) {
            case MODE_HOLD_ON_LAST_FRAME:
                model->setAnimationFrameNo(data.endframe);
                break;
            case MODE_ONCE: {
                auto it = data.transitions.find(TRIGGERTYPE_AFTER_COMPLETION);
                if (it != data.transitions.end()) {
                    nextanimation = it->second.targetanimation;
                }
                model->setAnimationFrameNo(data.endframe);
                break;
            }
            case MODE_REPEAT:
                int length = data.endframe - data.startframe + 1;
                if (length <= 0) {
                    length = 1;
                }
                newframe = data.startframe + ((newframe - data.startframe) % length);
                model->setAnimationFrameNo(newframe);
                break;
        }
    } else {
        model->setAnimationFrameNo(newframe);
    }
}