Texture2D sceneTexture : register(t0);
Texture2D bloomTexture : register(t1);
SamplerState SampleType : register(s0);

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

float4 main(VS_OUTPUT input) : SV_Target
{
    float4 sceneColor = sceneTexture.Sample(SampleType, input.Tex);
    float4 bloomColor = bloomTexture.Sample(SampleType, input.Tex);

    // 元のシーンの色とブルームの色を加算合成
    float4 finalColor = sceneColor + bloomColor;

    return finalColor;
}