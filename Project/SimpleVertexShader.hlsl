// SimpleVertexShader.hlsl (この内容で完全に置き換えてください)

cbuffer MatrixBuffer : register(b0)
{
    matrix WorldMatrix;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    matrix WorldInverseTransposeMatrix;
    matrix LightViewMatrix;
    matrix LightProjectionMatrix;
};

// アプリケーションからの入力頂点データ
struct VS_INPUT
{
    float4 Pos : POSITION;
    // 注: このシェーダーはPOSITIONしか使いませんが、入力構造は他のシェーダーと合わせます
    float4 Color : COLOR;
    float2 Tex : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
};

// ピクセルシェーダーへの出力
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    float4 pos = input.Pos;

    // ワールド、ビュー、プロジェクション行列を適用して座標を変換
    pos = mul(pos, WorldMatrix);
    pos = mul(pos, ViewMatrix);
    pos = mul(pos, ProjectionMatrix);
    output.Pos = pos;
    
    return output;
}