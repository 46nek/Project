// 定数バッファ: CPUからGPUへ行列データを渡す
cbuffer MatrixBuffer : register(b0)
{
    matrix WorldMatrix;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    matrix WorldInverseTransposeMatrix;
    matrix LightViewMatrix;
    matrix LightProjectionMatrix;
    matrix previousViewProjection;
};

// 頂点シェーダーへの入力データ構造
struct VS_INPUT
{
    float4 Pos : POSITION; // 頂点座標
    float4 Color : COLOR; // 頂点カラー
    float2 Tex : TEXCOORD0; // テクスチャ座標
    float3 Normal : NORMAL; // 法線ベクトル
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
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
    float4 PrevPos : TEXCOORD2; // 1フレーム前のスクリーン座標
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
};

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;

    // 現在のフレームの座標変換
    float4 worldPos = mul(input.Pos, WorldMatrix);
    output.WorldPos = worldPos.xyz;
    float4 viewPos = mul(worldPos, ViewMatrix);
    output.Pos = mul(viewPos, ProjectionMatrix);
    
    // 法線、接線、従法線をワールド空間に変換
    output.Normal = normalize(mul(input.Normal, (float3x3) WorldInverseTransposeMatrix));
    output.Tangent = normalize(mul(input.Tangent, (float3x3) WorldInverseTransposeMatrix));
    output.Binormal = normalize(mul(input.Binormal, (float3x3) WorldInverseTransposeMatrix));

    // シャドウマッピングのための座標変換
    output.LightViewPos = mul(worldPos, LightViewMatrix);
    output.LightViewPos = mul(output.LightViewPos, LightProjectionMatrix);

    // 1フレーム前のスクリーン座標を計算
    output.PrevPos = mul(worldPos, previousViewProjection);

    // テクスチャ座標とカラーをそのままピクセルシェーダーへ渡す
    output.Tex = input.Tex;
    output.Color = input.Color;

    return output;
}