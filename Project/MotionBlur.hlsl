Texture2D sceneTexture : register(t0); // 描画されたシーン
Texture2D velocityTexture : register(t1); // 速度マップ
SamplerState Sampler : register(s0);

static const int NUM_SAMPLES = 12; // ブラーの品質（大きいほど高品質だが重い）

float4 MotionBlurPS(float4 pos : SV_POSITION, float2 tex : TEXCOORD) : SV_Target
{
    // 速度マップからこのピクセルの速度ベクトルを読み取る
    float2 velocity = velocityTexture.Sample(Sampler, tex).xy;

    // 速度がほぼゼロならブラーをかけずに元の色を返す
    if (length(velocity) < 0.0001f)
    {
        return sceneTexture.Sample(Sampler, tex);
    }

    float4 finalColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // 速度ベクトルの方向に沿って、テクスチャを複数回サンプリングする
    for (int i = 0; i < NUM_SAMPLES; ++i)
    {
        float scale = (float) i / (float) (NUM_SAMPLES - 1);
        finalColor += sceneTexture.Sample(Sampler, tex + velocity * scale);
    }

    // 平均を取ってブラー色を決定
    return finalColor / NUM_SAMPLES;
}