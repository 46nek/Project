Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

// 明るさのしきい値
static const float brightnessThreshold = 0.8f;

float4 main(VS_OUTPUT input) : SV_Target
{
    float4 color = shaderTexture.Sample(SampleType, input.Tex);

    // ピクセルの輝度を計算
    float luminance = dot(color.rgb, float3(0.2126, 0.7152, 0.0722));

    // 輝度がしきい値より低い場合は黒にする
    if (luminance < brightnessThreshold)
    {
        color.rgb = float3(0.0f, 0.0f, 0.0f);
    }

    return color;
}