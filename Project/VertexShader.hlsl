// 定数バッファ: CPUからGPUへ行列データを渡す
cbuffer MatrixBuffer : register(b0)
{
    matrix WorldMatrix;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    matrix WorldInverseTransposeMatrix;
    matrix LightViewMatrix;
    matrix LightProjectionMatrix;
};

// 頂点シェーダーへの入力データ構造
struct VS_INPUT
{
    float4 Pos : POSITION; // 頂点座標
    float4 Color : COLOR; // 頂点カラー
    float2 Tex : TEXCOORD0; // テクスチャ座標
    float3 Normal : NORMAL; // 法線ベクトル
};

// 頂点シェーダーからピクセルシェーダーへの出力データ構造
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION; // スクリーン上の座標
    float4 Color : COLOR; // 頂点カラー
    float2 Tex : TEXCOORD0; // テクスチャ座標
    float3 Normal : NORMAL; // 法線ベクトル
    float3 WorldPos : WORLDPOS; // ワールド空間での座標
    float4 LightViewPos : TEXCOORD1; // ライトから見た座標
};

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    
    // 座標変換
    float4 worldPos = mul(input.Pos, WorldMatrix);
    output.WorldPos = worldPos.xyz;
    float4 viewPos = mul(worldPos, ViewMatrix);
    output.Pos = mul(viewPos, ProjectionMatrix);

    // 法線ベクトルをワールド空間に変換
    output.Normal = mul(input.Normal, (float3x3) WorldInverseTransposeMatrix);
    output.Normal = normalize(output.Normal);

    // シャドウマッピングのための座標変換
    output.LightViewPos = mul(worldPos, LightViewMatrix);
    output.LightViewPos = mul(output.LightViewPos, LightProjectionMatrix);
    
    // テクスチャ座標とカラーをそのままピクセルシェーダーへ渡す
    output.Tex = input.Tex;
    output.Color = input.Color;

    return output;
}