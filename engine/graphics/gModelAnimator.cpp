//
// Created by Metehan Gezer on 10/05/2025.
//

#include "gModelAnimator.h"
#include "gAppManager.h"

gModelAnimator::gModelAnimator() {

}

gModelAnimator::~gModelAnimator() {

}

void gModelAnimator::addAnimation(int id, int startframe, int endframe, AnimationMode mode, bool isDefault) {
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

void gModelAnimator::addSwitch(AnimationSwitch switchMode, int srcAnimation, int dstAnimation) {
    animations[srcAnimation].switches.emplace(
            switchMode,
            SwitchData{
                    .targetanimation = dstAnimation
            }
    );
}

void gModelAnimator::bake() {
    if (animations.empty()) {
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
    if (animations.empty()) {
        return;
    }
    double deltatime = appmanager->getElapsedTime();
    nextframetime += 65.0f / speed * deltatime;
    std::cout << nextframetime << ", " << deltatime << std::endl;
    if (nextframetime < 1.0f) {
        return;
    }
    if (nextanimation != currentanimation) {
        currentanimation = nextanimation;
        prepareAnimation(currentanimation);
    }
    auto it = animations.find(currentanimation);
    if (it == animations.end()) {
        return;
    }
    AnimationData& data = it->second;
    int currentframe = model->getAnimationFrameNo();
    if (currentframe >= data.endframe) {
        switch (data.mode) {
            case ANIMATIONMODE_HOLD_ON_LAST_FRAME:
                return;
            case ANIMATIONMODE_ONCE: {
                auto it = data.switches.find(ANIMATIONSWITCH_AFTER_COMPLETION);
                if (it != data.switches.end()) {
                    nextanimation = it->second.targetanimation;
                }
                return;
            }
            case ANIMATIONMODE_REPEAT:
                model->setAnimationFrameNo(data.startframe);
                return;
        }
    } else if (currentframe < data.startframe) {
        model->setAnimationFrameNo(data.startframe);
    }
    model->nextAnimationFrame();
    nextframetime = 0.0f;
}