struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    // 頂点座標とテクスチャ座標をそのまま渡す
    output.Pos = input.Pos;
    output.Tex = input.Tex;
    return output;
}