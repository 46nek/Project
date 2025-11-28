#pragma once

#include <cmath>
#include <algorithm>

/**
 * @brief イージングの種類を定義する列挙型
 */
enum class EasingType
{
    Linear,
    EaseInQuad, EaseOutQuad, EaseInOutQuad,
    EaseInCubic, EaseOutCubic, EaseInOutCubic,
    EaseInQuart, EaseOutQuart, EaseInOutQuart,
    EaseInQuint, EaseOutQuint, EaseInOutQuint,
    EaseInSine, EaseOutSine, EaseInOutSine,
    EaseInExpo, EaseOutExpo, EaseInOutExpo,
    EaseInCirc, EaseOutCirc, EaseInOutCirc,
    EaseInBack, EaseOutBack, EaseInOutBack,
    EaseInElastic, EaseOutElastic, EaseInOutElastic,
    EaseInBounce, EaseOutBounce, EaseInOutBounce
};

/**
 * @brief イージング計算を行う静的クラス
 */
class Easing
{
public:
    /**
     * @brief 指定されたイージングタイプに基づいて補間値を計算します。
     * @param type イージングの種類
     * @param t 進行度 (0.0f ～ 1.0f)
     * @return イージング適用後の値 (概ね 0.0f ～ 1.0f)
     */
    static float GetValue(EasingType type, float t);

private:
    // 定数
    static constexpr float PI = 3.14159265359f;

    // 各イージング関数の実装
    static float EaseInQuad(float t);
    static float EaseOutQuad(float t);
    static float EaseInOutQuad(float t);

    static float EaseInCubic(float t);
    static float EaseOutCubic(float t);
    static float EaseInOutCubic(float t);

    static float EaseInQuart(float t);
    static float EaseOutQuart(float t);
    static float EaseInOutQuart(float t);

    static float EaseInQuint(float t);
    static float EaseOutQuint(float t);
    static float EaseInOutQuint(float t);

    static float EaseInSine(float t);
    static float EaseOutSine(float t);
    static float EaseInOutSine(float t);

    static float EaseInExpo(float t);
    static float EaseOutExpo(float t);
    static float EaseInOutExpo(float t);

    static float EaseInCirc(float t);
    static float EaseOutCirc(float t);
    static float EaseInOutCirc(float t);

    static float EaseInBack(float t);
    static float EaseOutBack(float t);
    static float EaseInOutBack(float t);

    static float EaseInElastic(float t);
    static float EaseOutElastic(float t);
    static float EaseInOutElastic(float t);

    static float EaseInBounce(float t);
    static float EaseOutBounce(float t);
    static float EaseInOutBounce(float t);
};