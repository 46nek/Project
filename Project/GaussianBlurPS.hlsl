Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

cbuffer BlurBuffer : register(b0)
{
    float2 textureSize;
    float2 blurDirection; // (1, 0) for horizontal, (0, 1) for vertical
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

float4 main(VS_OUTPUT input) : SV_Target
{
    float4 finalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float weights[9] = { 0.05, 0.09, 0.12, 0.15, 0.18, 0.15, 0.12, 0.09, 0.05 };
    float2 texelSize = 1.0f / textureSize;

    for (int i = -4; i <= 4; i++)
    {
        float2 offset = blurDirection * texelSize * i;
        finalColor += shaderTexture.Sample(SampleType, input.Tex + offset) * weights[i + 4];
    }

    return finalColor;
}