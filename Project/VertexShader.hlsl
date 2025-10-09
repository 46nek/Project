// VertexShader.hlsl

// C++のMatrixBufferType構造体と一致させる
cbuffer MatrixBuffer : register(b0)
{
    matrix WorldMatrix;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
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
};

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    
    // 頂点位置の変換
    float4 pos = mul(input.Pos, WorldMatrix);
    pos = mul(pos, ViewMatrix);
    pos = mul(pos, ProjectionMatrix);
    output.Pos = pos;

    // 法線をワールド空間に変換してピクセルシェーダーに渡す
    output.Normal = mul(input.Normal, (float3x3) WorldMatrix);
    output.Normal = normalize(output.Normal);

    // テクスチャ座標と色を渡す
    output.Tex = input.Tex;
    output.Color = input.Color;

    return output;
}