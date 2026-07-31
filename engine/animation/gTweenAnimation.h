/*
 * gTweenAnimation.h
 *
 *  Created on: 17 Jul 2026
 *      Author: Remzi ISCI
 */

#ifndef G_TWEEN_ANIMATION_H
#define G_TWEEN_ANIMATION_H

#include <cmath>
#include <functional>


#ifndef M_PI_F
#define M_PI_F 3.14159265358979323846f
#endif

#ifndef M_PI_2_F
#define M_PI_2_F 1.57079632679489661923f
#endif

// Easing functions
static inline float EaseLinear(float t) { return t; }

static inline float EaseIn(float t) { return t * t; }

static inline float EaseOut(float t) { return t * (2.0f - t); }

static inline float EaseInOut(float t) {
    return (t < 0.5f) ? (2.0f * t * t) : (-1.0f + (4.0f - 2.0f * t) * t);
}

static inline float EaseInCubic(float t) { return t * t * t; }

static inline float EaseOutCubic(float t) {
    const float tMinusOne = t - 1.0f;
    return tMinusOne * tMinusOne * tMinusOne + 1.0f;
}

static inline float EaseInOutCubic(float t) {
    if (t < 0.5f) return 4.0f * t * t * t;
    const float tMinusOne = t - 1.0f;
    const float doubleTMinusTwo = 2.0f * t - 2.0f;
    return tMinusOne * doubleTMinusTwo * doubleTMinusTwo + 1.0f;
}

static inline float EaseInQuart(float t) { return t * t * t * t; }

static inline float EaseOutQuart(float t) {
    const float tMinusOne = t - 1.0f;
    return 1.0f - (tMinusOne * tMinusOne * tMinusOne * tMinusOne);
}

static inline float EaseInOutQuart(float t) {
    if (t < 0.5f) return 8.0f * t * t * t * t;
    const float tMinusOne = t - 1.0f;
    return 1.0f - 8.0f * tMinusOne * tMinusOne * tMinusOne * tMinusOne;
}

static inline float EaseInQuint(float t) { return t * t * t * t * t; }

static inline float EaseOutQuint(float t) {
    const float tMinusOne = t - 1.0f;
    return 1.0f + (tMinusOne * tMinusOne * tMinusOne * tMinusOne * tMinusOne);
}

static inline float EaseInOutQuint(float t) {
    const float tMinusOne = t - 1.0f;
    if (t < 0.5f) return 16.0f * t * t * t * t * t;
    return 1.0f + 16.0f * tMinusOne * tMinusOne * tMinusOne * tMinusOne * tMinusOne;
}

static inline float EaseInSine(float t) { return 1.0f - cosf(t * M_PI_2_F); }

static inline float EaseOutSine(float t) { return sinf(t * M_PI_2_F); }

static inline float EaseInOutSine(float t) { return 0.5f * (1.0f - cosf(M_PI_F * t)); }

static inline float EaseInExpo(float t) { return (t == 0.0f) ? 0.0f : powf(2.0f, 10.0f * (t - 1.0f)); }

static inline float EaseOutExpo(float t) { return (t == 1.0f) ? 1.0f : 1.0f - powf(2.0f, -10.0f * t); }

static inline float EaseInOutExpo(float t) {
    if (t == 0.0f) return 0.0f;
    if (t == 1.0f) return 1.0f;
    if (t < 0.5f) return 0.5f * powf(2.0f, 20.0f * t - 10.0f);
    return 1.0f - 0.5f * powf(2.0f, -20.0f * t + 10.0f);
}

static inline float EaseInCircular(float t) { return 1.0f - sqrtf(1.0f - t * t); }

static inline float EaseOutCircular(float t) {
    const float tMinusOne = t - 1.0f;
    return sqrtf(1.0f - (tMinusOne * tMinusOne));
}

static inline float EaseInOutCircular(float t) {
    if (t < 0.5f) return 0.5f * (1.0f - sqrtf(1.0f - 4.0f * t * t));
    const float tMinusOne = t - 1.0f;
    return 0.5f * (sqrtf(1.0f - 4.0f * tMinusOne * tMinusOne) + 1.0f);
}

static inline float EaseInBack(float t) {
    const float s = 1.70158f;
    return t * t * ((s + 1.0f) * t - s);
}

static inline float EaseOutBack(float t) {
    const float s = 1.70158f;
    const float tMinusOne = t - 1.0f;
    return tMinusOne * tMinusOne * ((s + 1.0f) * tMinusOne + s) + 1.0f;
}

static inline float EaseInOutBack(float t) {
    const float s = 1.70158f;
    if (t < 0.5f) return 0.5f * (t * t * ((s + 1.0f) * t - s));
    const float tMinusOne = t - 1.0f;
    return 0.5f * (tMinusOne * tMinusOne * ((s + 1.0f) * tMinusOne + s) + 2.0f);
}

static inline float EaseOutBounce(float t) {
    if (t < (1.0f / 2.75f)) {
        return 7.5625f * t * t;
    } else if (t < (2.0f / 2.75f)) {
        const float tTemp = t - (1.5f / 2.75f);
        return 7.5625f * tTemp * tTemp + 0.75f;
    } else if (t < (2.5f / 2.75f)) {
        const float tTemp = t - (2.25f / 2.75f);
        return 7.5625f * tTemp * tTemp + 0.9375f;
    } else {
        const float tTemp = t - (2.625f / 2.75f);
        return 7.5625f * tTemp * tTemp + 0.984375f;
    }
}

static inline float EaseInBounce(float t) { return 1.0f - EaseOutBounce(1.0f - t); }

static inline float EaseInOutBounce(float t) {
    if (t < 0.5f) return 0.5f * EaseInBounce(t * 2.0f);
    return 0.5f * EaseOutBounce(t * 2.0f - 1.0f) + 0.5f;
}

class gTweenAnimation {
public:
    enum EASE_TYPE : int {
        EASE_LINEAR = 0, EASE_IN, EASE_OUT, EASE_IN_OUT,
        EASE_IN_CUBIC, EASE_OUT_CUBIC, EASE_IN_OUT_CUBIC,
        EASE_IN_QUART, EASE_OUT_QUART, EASE_IN_OUT_QUART,
        EASE_IN_QUINT, EASE_OUT_QUINT, EASE_IN_OUT_QUINT,
        EASE_IN_SINE, EASE_OUT_SINE, EASE_IN_OUT_SINE,
        EASE_IN_EXPO, EASE_OUT_EXPO, EASE_IN_OUT_EXPO,
        EASE_IN_CIRCULAR, EASE_OUT_CIRCULAR, EASE_IN_OUT_CIRCULAR,
        EASE_IN_BACK, EASE_OUT_BACK, EASE_IN_OUT_BACK,
        EASE_IN_BOUNCE, EASE_OUT_BOUNCE, EASE_IN_OUT_BOUNCE
    };

    float startvalue = 0.0f;
    float endvalue = 0.0f;
    float duration = 0.0f;
    float elapsedtime = 0.0f;
    float delay = 0.0f;
    float delaytimer = 0.0f;
    //Target valu will be changed by tween system
    float *targetptr = nullptr;
    int repeatcount = 0;
    bool isactive = false;
    bool isfinished = false;
    std::function<void()> FinishCallback = nullptr;

    gTweenAnimation();
    ~gTweenAnimation();

    void Set(float startvalue, float endvalue, float duration, float *targetptr, EASE_TYPE easetype = EASE_LINEAR, std::function<void()> FinishCallback = nullptr, float delay = 0.0f);
    void setDelay(float delay);
    float getDelay() const;
    bool isDelaying() const;
    void SetEasetype(EASE_TYPE easetype);
    void Update(float deltaTime);
    void Destroy();
    void Reset(int repeatCount);
    //-1 means work forever
    void Start(int repeatCount);
    void Stop();

private:
    float (*easeFunc)(float) = nullptr;
    EASE_TYPE easetype = EASE_TYPE::EASE_LINEAR;
    void AssignEaseFunction();
};

#endif /* G_TWEEN_ANIMATION_H */
