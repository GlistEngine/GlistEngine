//
// Created by Metehan Gezer on 10/05/2025.
//

#ifndef GRAPHICS_GMODELANIMATOR_H
#define GRAPHICS_GMODELANIMATOR_H

#include "gModel.h"

class gModelAnimator {
public:
    gModelAnimator();
    ~gModelAnimator();

    enum AnimationMode {
        ANIMATIONMODE_HOLD_ON_LAST_FRAME,
        ANIMATIONMODE_ONCE,
        ANIMATIONMODE_REPEAT
    };

    enum AnimationSwitch {
        ANIMATIONSWITCH_AFTER_COMPLETION,
    };

    void setModel(gModel* model) {
        this->model = model;
    }

    void addAnimation(int id, int startframe, int endframemode, AnimationMode mode, bool isDefault = false);
    void addSwitch(AnimationSwitch switchMode, int srcAnimation, int dstAnimation);
    void bake();

    void setGeneralSpeed(float speed) {
        this->speed = speed;
    }

    void triggerAnimation(int animationId);
    void update();


private:
    struct SwitchData {
        int targetanimation;
    };
    struct AnimationData {
        int id;
        AnimationMode mode;
        int startframe;
        int endframe;
        std::unordered_map<AnimationSwitch, SwitchData> switches;

    };

    std::unordered_map<int, AnimationData> animations;
    int defaultanimation = 0;
    int maxframenum = 0;

    gModel* model;
    int currentanimation;
    int nextanimation;
    float nextframetime = 0;
    float speed = 1.0f;

    void prepareAnimation(int animationId);
};


#endif //GRAPHICS_GMODELANIMATOR_H
