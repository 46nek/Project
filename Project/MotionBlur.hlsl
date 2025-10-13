Texture2D sceneTexture : register(t0);
Texture2D depthTexture : register(t1);
SamplerState Sampler : register(s0);

cbuffer MotionBlurBuffer : register(b0)
{
    matrix previousViewProjection;
    matrix currentViewProjectionInverse;
    float blurAmount;
    float3 padding;
};

float4 main(float4 position : SV_POSITION, float2 tex : TEXCOORD) : SV_TARGET
{
    // 深度テクスチャから深度値を取得
    float depth = depthTexture.Sample(Sampler, tex).r;

    // 深度が1.0（空）の場合はブラーを適用しない
    if (depth >= 1.0f)
    {
        return sceneTexture.Sample(Sampler, tex);
    }

    // 現在のピクセルの位置をNDC（正規化デバイス座標）からビュー空間へ変換
    float4 currentPosH;
    currentPosH.x = tex.x * 2.0f - 1.0f;
    currentPosH.y = (1.0f - tex.y) * 2.0f - 1.0f;
    currentPosH.z = depth;
    currentPosH.w = 1.0f;

    // ワールド座標に変換
    currentPosH = mul(currentPosH, currentViewProjectionInverse);
    currentPosH /= currentPosH.w;

    // 前のフレームでのスクリーン座標を計算
    float4 previousPosH = mul(currentPosH, previousViewProjection);
    previousPosH /= previousPosH.w;

    // 前のフレームと現在のフレームのスクリーン座標の差分から速度を計算
    float2 velocity = (currentPosH.xy - previousPosH.xy) * blurAmount;

    // シーンテクスチャを複数回サンプリングしてブラー効果を生成
    float4 finalColor = 0;
    const int SAMPLES = 16; // サンプリング回数
    for (int i = 0; i < SAMPLES; ++i)
    {
        finalColor += sceneTexture.Sample(Sampler, tex + velocity * (float(i) / float(SAMPLES - 1)));
    }

    // 平均を取る
    finalColor /= SAMPLES;

    return finalColor;
}