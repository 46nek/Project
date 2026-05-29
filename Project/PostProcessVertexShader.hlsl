// 3Dモデルと同じ、完全な頂点構造を入力として受け取る
struct VS_INPUT
{
    float4 Pos : POSITION;
    float4 Color : COLOR;
    float2 Tex : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    // 受け取ったデータのうち、位置とテクスチャ座標だけをピクセルシェーダーへ渡す
    output.Pos = input.Pos;
    output.Tex = input.Tex;
    return output;
}