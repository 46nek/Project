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
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;

    // 頂点座標を行列で変換
    output.Pos = mul(input.Pos, WorldMatrix);
    output.Pos = mul(output.Pos, ViewMatrix);
    output.Pos = mul(output.Pos, ProjectionMatrix);

    // 色はそのまま渡す
    output.Color = input.Color;

    return output;
}