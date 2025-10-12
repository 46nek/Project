Texture2D shaderTexture : register(t0);
SamplerState Sampler : register(s0);

cbuffer ScreenSizeBuffer : register(b0)
{
    float screenWidth;
    float screenHeight;
};

static const int BLUR_RADIUS = 7;
static const float WEIGHTS[BLUR_RADIUS] = { 0.227027, 0.1945946, 0.1216216, 0.05405405, 0.01621622, 0.005405405, 0.001621622 };

float4 VerticalBlurPS(float4 pos : SV_POSITION, float2 tex : TEXCOORD) : SV_Target
{
    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float2 texelSize = float2(1.0f / screenWidth, 1.0f / screenHeight);

    color += shaderTexture.Sample(Sampler, tex) * WEIGHTS[0];

    for (int i = 1; i < BLUR_RADIUS; ++i)
    {
        color += shaderTexture.Sample(Sampler, tex + float2(0.0f, texelSize.y * i)) * WEIGHTS[i];
        color += shaderTexture.Sample(Sampler, tex - float2(0.0f, texelSize.y * i)) * WEIGHTS[i];
    }

    return color;
}