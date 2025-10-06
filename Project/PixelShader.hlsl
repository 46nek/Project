// 頂点シェーダーとピクセルシェーダーの間でデータを渡すための構造体
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION; // SV_POSITION は必須。頂点の最終的な座標
    float4 Color : COLOR; // COLOR は任意。ピクセルシェーダーに渡す色
};

// ピクセルシェーダー
// 頂点シェーダーから受け取った情報をもとに、ピクセルの最終的な色を決定します
float4 PS(VS_OUTPUT input) : SV_Target
{
    return input.Color; // 頂点の色をそのまま出力
}