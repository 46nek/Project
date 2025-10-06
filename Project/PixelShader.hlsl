// テクスチャとサンプラーを定義
Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

// 頂点シェーダーから渡されるデータの構造体
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 Tex : TEXCOORD0;
};

// ピクセルシェーダー本体
float4 PS(VS_OUTPUT input) : SV_Target
{
    //  サンプラーを使ってテクスチャからピクセルの色を取得する
    return shaderTexture.Sample(SampleType, input.Tex);
}