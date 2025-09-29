// 頂点シェーダーとピクセルシェーダーの間でデータを渡すための構造体
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION; // SV_POSITION は必須。頂点の最終的な座標
    float4 Color : COLOR; // COLOR は任意。ピクセルシェーダーに渡す色
};

// 頂点シェーダー
// C++から受け取った頂点データを処理し、ピクセルシェーダーに渡します
VS_OUTPUT VS(float4 Pos : POSITION, float4 Color : COLOR)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.Pos = Pos;
    output.Color = Color;
    return output;
}

// ピクセルシェーダー
// 頂点シェーダーから受け取った情報をもとに、ピクセルの最終的な色を決定します
float4 PS(VS_OUTPUT input) : SV_Target
{
    return input.Color; // 頂点の色をそのまま出力
}