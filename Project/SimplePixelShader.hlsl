cbuffer MaterialBuffer : register(b2)
{
    float4 EmissiveColor;
    bool UseTexture;
    bool UseNormalMap;
    float2 Padding;
};

float4 main() : SV_TARGET
{
    // マテリアルバッファから自己発光色を受け取り、そのまま出力する
    return EmissiveColor;
}