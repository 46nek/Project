cbuffer MatrixBuffer : register(b0)
{
    matrix WorldMatrix;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    matrix WorldInverseTransposeMatrix; 
};

struct VS_INPUT
{
    float4 Pos : POSITION;
    float4 Color : COLOR;
    float2 Tex : TEXCOORD0;
    float3 Normal : NORMAL;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 Tex : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 WorldPos : WORLDPOS;
};

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    
    float4 worldPos = mul(input.Pos, WorldMatrix);
    output.WorldPos = worldPos.xyz;

    float4 viewPos = mul(worldPos, ViewMatrix);
    output.Pos = mul(viewPos, ProjectionMatrix);
    
    output.Normal = mul(input.Normal, (float3x3) WorldInverseTransposeMatrix);
    output.Normal = normalize(output.Normal);
    
    output.Tex = input.Tex;
    output.Color = input.Color;

    return output;
}