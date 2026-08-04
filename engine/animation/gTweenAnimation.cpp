/*
 * gTweenAnimation.cpp
 *
 *  Created on: 17 Jul 2026
 *      Author: Remzi ISCI
 */

#include "gTweenAnimation.h"
#include "gUtils.h"

gTweenAnimation::gTweenAnimation() {}
gTweenAnimation::~gTweenAnimation() {}

void gTweenAnimation::Set(float startvalue, float endvalue, float duration, float *targetptr, EASE_TYPE easetype, std::function<void()> FinishCallback, float delay) {
    this->startvalue = startvalue;
    this->endvalue = endvalue;
    this->duration = (duration <= 0.0f) ? 0.001f : duration; // To prevent to devide to 0 bug
    this->delay = (delay < 0.0f) ? 0.0f : delay;
    this->delaytimer = this->delay;
    this->elapsedtime = 0.0f;
    this->targetptr = targetptr;
    this->easetype = easetype;
    this->FinishCallback = FinishCallback;
    this->isactive = false;
    this->isfinished = false;
    AssignEaseFunction();
}

void gTweenAnimation::setDelay(float delay) {
    this->delay = (delay < 0.0f) ? 0.0f : delay;
    this->delaytimer = this->delay;
}

float gTweenAnimation::getDelay() const {
    return delay;
}

bool gTweenAnimation::isDelaying() const {
    return isactive && (delaytimer > 0.0f);
}

void gTweenAnimation::SetEasetype(EASE_TYPE easetype) {
    this->easetype = easetype;
    AssignEaseFunction();
}

void gTweenAnimation::Update(float deltatime) {
    if (!isactive || isfinished) return;

    if (delaytimer > 0.0f) {
        delaytimer -= deltatime;
        if (delaytimer > 0.0f) {
            return;
        }
        delaytimer = 0.0f;
    }

    elapsedtime += deltatime;
    // isfinished will be setted true when duration is reached
    if (elapsedtime >= duration) {
        elapsedtime = duration;
        isfinished = true;
    }

    float t = elapsedtime / duration;
    if (easeFunc != nullptr) {
        t = easeFunc(t);
    }

    if (targetptr != nullptr) {
        *targetptr = startvalue + t * (endvalue - startvalue);
    } else {
    	 gLogi("gTweenAnimation") << "A tween animation is working without target ptr";
    }

    if (isfinished) {
        if (repeatcount == -1) {
            //Unlimited cycle
            elapsedtime = 0.0f;
            delaytimer = delay;
            isfinished = false;
        } else if (repeatcount > 1) {
            //Decrease work count time
            repeatcount--;
            elapsedtime = 0.0f;
            delaytimer = delay;
            isfinished = false;
        } else {
            // repeatcount == 0 (Animasyon is finished)
            isactive = false;
        }
        if (FinishCallback != nullptr) {
            FinishCallback();
        }
    }
}

void gTweenAnimation::AssignEaseFunction() {
    switch (easetype) {
        case EASE_LINEAR:          easeFunc = EaseLinear; break;
        case EASE_IN:              easeFunc = EaseIn; break;
        case EASE_OUT:             easeFunc = EaseOut; break;
        case EASE_IN_OUT:          easeFunc = EaseInOut; break;
        case EASE_IN_CUBIC:        easeFunc = EaseInCubic; break;
        case EASE_OUT_CUBIC:       easeFunc = EaseOutCubic; break;
        case EASE_IN_OUT_CUBIC:    easeFunc = EaseInOutCubic; break;
        case EASE_IN_QUART:        easeFunc = EaseInQuart; break;
        case EASE_OUT_QUART:       easeFunc = EaseOutQuart; break;
        case EASE_IN_OUT_QUART:    easeFunc = EaseInOutQuart; break;
        case EASE_IN_QUINT:        easeFunc = EaseInQuint; break;
        case EASE_OUT_QUINT:       easeFunc = EaseOutQuint; break;
        case EASE_IN_OUT_QUINT:    easeFunc = EaseInOutQuint; break;
        case EASE_IN_SINE:         easeFunc = EaseInSine; break;
        case EASE_OUT_SINE:        easeFunc = EaseOutSine; break;
        case EASE_IN_OUT_SINE:     easeFunc = EaseInOutSine; break;
        case EASE_IN_EXPO:         easeFunc = EaseInExpo; break;
        case EASE_OUT_EXPO:        easeFunc = EaseOutExpo; break;
        case EASE_IN_OUT_EXPO:     easeFunc = EaseInOutExpo; break;
        case EASE_IN_CIRCULAR:     easeFunc = EaseInCircular; break;
        case EASE_OUT_CIRCULAR:    easeFunc = EaseOutCircular; break;
        case EASE_IN_OUT_CIRCULAR: easeFunc = EaseInOutCircular; break;
        case EASE_IN_BACK:         easeFunc = EaseInBack; break;
        case EASE_OUT_BACK:        easeFunc = EaseOutBack; break;
        case EASE_IN_OUT_BACK:     easeFunc = EaseInOutBack; break;
        case EASE_IN_BOUNCE:       easeFunc = EaseInBounce; break;
        case EASE_OUT_BOUNCE:      easeFunc = EaseOutBounce; break;
        case EASE_IN_OUT_BOUNCE:   easeFunc = EaseInOutBounce; break;
        default:
            easeFunc = EaseLinear;
            gLogi("gTweenAnimation") << "Unsupported ease type: " << easetype << ". Fallback to Linear.";
            break;
    }
}

void gTweenAnimation::Destroy() {}

void gTweenAnimation::Reset(int repeatcount) {
    elapsedtime = 0.0f;
    delaytimer = delay;
    isfinished = false;
    this->repeatcount = repeatcount;
    if (targetptr != nullptr) {
        *targetptr = startvalue;
    }
    isactive = false;
}

void gTweenAnimation::Start(int repeatcount) {
    isactive = true;
    elapsedtime = 0.0f;
    delaytimer = delay;
    isfinished = false;
    this->repeatcount = repeatcount;
    if (targetptr != nullptr) {
        *targetptr = startvalue;
    } else {
        gLogi("gTweenAnimation") << "Target pointer is NULL on Start!";
    }
}

void gTweenAnimation::Stop() {
    isactive = false;
}
