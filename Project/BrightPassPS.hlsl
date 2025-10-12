Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

// ���邳�̂������l
static const float brightnessThreshold = 0.8f;

float4 main(VS_OUTPUT input) : SV_Target
{
    float4 color = shaderTexture.Sample(SampleType, input.Tex);

    // �s�N�Z���̋P�x���v�Z
    float luminance = dot(color.rgb, float3(0.2126, 0.7152, 0.0722));

    // �P�x���������l���Ⴂ�ꍇ�͍��ɂ���
    if (luminance < brightnessThreshold)
    {
        color.rgb = float3(0.0f, 0.0f, 0.0f);
    }

    return color;
}