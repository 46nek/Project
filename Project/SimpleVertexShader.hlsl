cbuffer MatrixBuffer : register(b0)
{
    matrix WorldMatrix;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    matrix WorldInverseTransposeMatrix;
    matrix LightViewMatrix;
    matrix LightProjectionMatrix;
};

struct VS_INPUT
{
    float4 Pos : POSITION;
    // 注: このシェーダーはPOSITIONしか使いませんが、入力構造は他のシェーダーと合わせます
    float4 Color : COLOR;
    float2 Tex : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
};

float4 main(VS_INPUT input) : SV_POSITION
{
    float4 pos = input.Pos;
    pos = mul(pos, WorldMatrix);
    pos = mul(pos, ViewMatrix);
    pos = mul(pos, ProjectionMatrix);
    return pos;
}