// VertexShader.hlsl

// C++‚ÌMatrixBufferType\‘¢‘Ì‚Æˆê’v‚³‚¹‚é
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
    
    // •ÏŠ·Œ‹‰Ê‚ğˆê•Ï”‚ÉŠi”[‚µA‡”Ô‚ÉæZ‚µ‚Ä‚¢‚­
    float4 pos = mul(input.Pos, WorldMatrix);
    pos = mul(pos, ViewMatrix);
    pos = mul(pos, ProjectionMatrix);
    output.Pos = pos;

    // F‚Í‚»‚Ì‚Ü‚Ü“n‚·
    output.Color = input.Color;

    return output;
}