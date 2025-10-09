// PixelShader.hlsl

Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

// ライトの情報を格納する新しいコンスタントバッファ
cbuffer LightBuffer : register(b1)
{
    // C++側の構造体と順番を合わせる
    float4 DiffuseColor;
    float3 LightDirection;
};


struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 Tex : TEXCOORD0;
    float3 Normal : NORMAL;
};

float4 PS(VS_OUTPUT input) : SV_Target
{
    float4 textureColor;
    float3 lightDir;
    float lightIntensity;
    float4 color;

    // テクスチャから色を取得
    textureColor = shaderTexture.Sample(SampleType, input.Tex);

    // ライトの方向ベクトルを反転
    lightDir = -normalize(LightDirection); //念のため正規化

    // 法線とライト方向の内積を計算して光の強度を決定
    lightIntensity = saturate(dot(input.Normal, lightDir));

    // 環境光（Ambient Light）を少しだけ加える
    float4 ambientColor = float4(0.65f, 0.65f, 0.65f, 1.0f);
    
    // 光の強度と色を最終的な色に乗算し、環境光を加算
    color = saturate((textureColor * DiffuseColor * lightIntensity) + (textureColor * ambientColor));


    return color;
}