cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix lightViewMatrix;
    matrix lightProjectionMatrix;
};

struct VS_INPUT
{
    float4 Pos : POSITION;
};

// ピクセルシェーダーにはSV_POSITION（スクリーン上の座標）のみを渡す
float4 main(VS_INPUT input) : SV_POSITION
{
    // 頂点座標をワールド、ライトビュー、ライトプロジェクションの順に変換する
    float4 worldPos = mul(input.Pos, worldMatrix);
    float4 viewPos = mul(worldPos, lightViewMatrix);
    float4 projPos = mul(viewPos, lightProjectionMatrix);

    return projPos;
}