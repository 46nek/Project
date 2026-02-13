#include "Easing.h"
#include <algorithm>
#include <cmath>

static const float PI = 3.1415926535f;

float Easing::GetValue(EasingType type, float t) {
    // t縺ｮ遽・峇繧・.0・・.0縺ｫ繧ｯ繝ｩ繝ｳ繝・
    t = std::max(0.0f, std::min(t, 1.0f));

    switch (type) {
    case EasingType::Linear: return t;
    case EasingType::EaseInQuad: return EaseInQuad(t);
    case EasingType::EaseOutQuad: return EaseOutQuad(t);
    case EasingType::EaseInOutQuad: return EaseInOutQuad(t);
    case EasingType::EaseInCubic: return EaseInCubic(t);
    case EasingType::EaseOutCubic: return EaseOutCubic(t);
    case EasingType::EaseInOutCubic: return EaseInOutCubic(t);
    case EasingType::EaseInQuart: return EaseInQuart(t);
    case EasingType::EaseOutQuart: return EaseOutQuart(t);
    case EasingType::EaseInOutQuart: return EaseInOutQuart(t);
    case EasingType::EaseInQuint: return EaseInQuint(t);
    case EasingType::EaseOutQuint: return EaseOutQuint(t);
    case EasingType::EaseInOutQuint: return EaseInOutQuint(t);
    case EasingType::EaseInSine: return EaseInSine(t);
    case EasingType::EaseOutSine: return EaseOutSine(t);
    case EasingType::EaseInOutSine: return EaseInOutSine(t);
    case EasingType::EaseInExpo: return EaseInExpo(t);
    case EasingType::EaseOutExpo: return EaseOutExpo(t);
    case EasingType::EaseInOutExpo: return EaseInOutExpo(t);
    case EasingType::EaseInCirc: return EaseInCirc(t);
    case EasingType::EaseOutCirc: return EaseOutCirc(t);
    case EasingType::EaseInOutCirc: return EaseInOutCirc(t);
    case EasingType::EaseInBack: return EaseInBack(t);
    case EasingType::EaseOutBack: return EaseOutBack(t);
    case EasingType::EaseInOutBack: return EaseInOutBack(t);
    case EasingType::EaseInElastic: return EaseInElastic(t);
    case EasingType::EaseOutElastic: return EaseOutElastic(t);
    case EasingType::EaseInOutElastic: return EaseInOutElastic(t);
    case EasingType::EaseInBounce: return EaseInBounce(t);
    case EasingType::EaseOutBounce: return EaseOutBounce(t);
    case EasingType::EaseInOutBounce: return EaseInOutBounce(t);
    default: return t;
    }
}

// --- Quad ---
float Easing::EaseInQuad(float t) { return t * t; }
float Easing::EaseOutQuad(float t) { return t * (2 - t); }
float Easing::EaseInOutQuad(float t) { return t < 0.5f ? 2 * t * t : -1 + (4 - 2 * t) * t; }

// --- Cubic ---
float Easing::EaseInCubic(float t) { return t * t * t; }
float Easing::EaseOutCubic(float t) { return (--t) * t * t + 1; }
float Easing::EaseInOutCubic(float t) { return t < 0.5f ? 4 * t * t * t : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1; }

// --- Quart ---
float Easing::EaseInQuart(float t) { return t * t * t * t; }
float Easing::EaseOutQuart(float t) { return 1 - (--t) * t * t * t; }
float Easing::EaseInOutQuart(float t) { return t < 0.5f ? 8 * t * t * t * t : 1 - 8 * (--t) * t * t * t; }

// --- Quint ---
float Easing::EaseInQuint(float t) { return t * t * t * t * t; }
float Easing::EaseOutQuint(float t) { return 1 + (--t) * t * t * t * t; }
float Easing::EaseInOutQuint(float t) { return t < 0.5f ? 16 * t * t * t * t * t : 1 + 16 * (--t) * t * t * t * t; }

// --- Sine ---
float Easing::EaseInSine(float t) { return 1 - cosf((t * PI) / 2); }
float Easing::EaseOutSine(float t) { return sinf((t * PI) / 2); }
float Easing::EaseInOutSine(float t) { return -(cosf(PI * t) - 1) / 2; }

// --- Expo ---
float Easing::EaseInExpo(float t) { return t == 0 ? 0 : powf(2, 10 * (t - 1)); }
float Easing::EaseOutExpo(float t) { return t == 1 ? 1 : 1 - powf(2, -10 * t); }
float Easing::EaseInOutExpo(float t) {
    if (t == 0) { return 0; }
    if (t == 1) { return 1; }
    if ((t *= 2) < 1) { return 0.5f * powf(2, 10 * (t - 1)); }
    return 0.5f * (-powf(2, -10 * --t) + 2);
}

// --- Circ ---
float Easing::EaseInCirc(float t) { return 1 - sqrtf(1 - t * t); }
float Easing::EaseOutCirc(float t) { return sqrtf(1 - (t - 1) * (t - 1)); }
float Easing::EaseInOutCirc(float t) {
    if ((t *= 2) < 1) { return -0.5f * (sqrtf(1 - t * t) - 1); }
    return 0.5f * (sqrtf(1 - (t - 2) * (t - 2)) + 1);
}

// --- Back ---
float Easing::EaseInBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1;
    return c3 * t * t * t - c1 * t * t;
}
float Easing::EaseOutBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1;
    return 1 + c3 * powf(t - 1, 3) + c1 * powf(t - 1, 2);
}
float Easing::EaseInOutBack(float t) {
    const float c1 = 1.70158f;
    const float c2 = c1 * 1.525f;
    return (t *= 2) < 1
        ? (powf(t, 2) * ((c2 + 1) * t - c2)) / 2
        : (powf(t - 2, 2) * ((c2 + 1) * (t - 2) + c2) + 2) / 2;
}

// --- Elastic ---
float Easing::EaseInElastic(float t) {
    const float c4 = (2 * PI) / 3;
    return t == 0 ? 0 : t == 1 ? 1 : -powf(2, 10 * t - 10) * sinf((t * 10 - 10.75f) * c4);
}
float Easing::EaseOutElastic(float t) {
    const float c4 = (2 * PI) / 3;
    return t == 0 ? 0 : t == 1 ? 1 : powf(2, -10 * t) * sinf((t * 10 - 0.75f) * c4) + 1;
}
float Easing::EaseInOutElastic(float t) {
    const float c5 = (2 * PI) / 4.5f;
    return t == 0 ? 0 : t == 1 ? 1 : t < 0.5f
        ? -(powf(2, 20 * t - 10) * sinf((20 * t - 11.125f) * c5)) / 2
        : (powf(2, -20 * t + 10) * sinf((20 * t - 11.125f) * c5)) / 2 + 1;
}

// --- Bounce ---
float Easing::EaseOutBounce(float t) {
    const float n1 = 7.5625f;
    const float d1 = 2.75f;
    if (t < 1 / d1) {
        return n1 * t * t;
    }
    else if (t < 2 / d1) {
        return n1 * (t -= 1.5f / d1) * t + 0.75f;
    }
    else if (t < 2.5f / d1) {
        return n1 * (t -= 2.25f / d1) * t + 0.9375f;
    }
    else {
        return n1 * (t -= 2.625f / d1) * t + 0.984375f;
    }
}
float Easing::EaseInBounce(float t) { return 1 - EaseOutBounce(1 - t); }
float Easing::EaseInOutBounce(float t) {
    return t < 0.5f ? (1 - EaseOutBounce(1 - 2 * t)) / 2 : (1 + EaseOutBounce(2 * t - 1)) / 2;
}

