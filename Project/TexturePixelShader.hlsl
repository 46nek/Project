Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 Tex : TEXCOORD0;
};

float4 PS(VS_OUTPUT input) : SV_TARGET
{
    // �e�N�X�`������F���T���v�����O���ĕԂ�
    return shaderTexture.Sample(SampleType, input.Tex);
}