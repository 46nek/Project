// MotionBlur.hlsl (修正版)

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

    // 何も描かれていないピクセル(空)は何もしない
    if (depth >= 1.0f)
    {
        return sceneTexture.Sample(Sampler, tex);
    }

    // --- 1. 現在のピクセルのワールド座標を復元 ---
    // スクリーン座標(tex)から-1～+1の座標系(NDC)に変換
    float2 ndc = float2(tex.x * 2.0f - 1.0f, (1.0f - tex.y) * 2.0f - 1.0f);
    float4 currentPosH = float4(ndc, depth, 1.0f);
    // 現在のカメラの情報を使って、ワールド座標に戻す
    currentPosH = mul(currentPosH, currentViewProjectionInverse);
    currentPosH /= currentPosH.w;

    // --- 2. そのワールド座標が「前のフレーム」では画面のどこにいたかを計算 ---
    float4 previousPosH = mul(currentPosH, previousViewProjection);
    previousPosH /= previousPosH.w;

    // --- 3.「現在」と「1フレーム前」のスクリーン座標の差分から動きのベクトルを計算 ---
    float2 velocity = (ndc - previousPosH.xy) / 2.0f;

    // ブラーの強さを適用
    velocity *= blurAmount;

    // --- 4. 動きのベクトルに沿って複数回テクスチャをサンプリングし、平均をとる ---
    float4 finalColor = 0;
    const int SAMPLES = 32; 
    for (int i = 0; i < SAMPLES; ++i)
    {
        finalColor += sceneTexture.Sample(Sampler, tex + velocity * (float(i) / (SAMPLES - 1)));
    }
    finalColor /= SAMPLES;

    return finalColor;
}