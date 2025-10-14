// テクスチャとサンプラーステート
Texture2D shaderTexture : register(t0); // オブジェクトのテクスチャ
Texture2D shadowMapTexture : register(t1); // シャドウマップ（深度テクスチャ）
Texture2D normalMapTexture : register(t2);
SamplerState SampleType : register(s0); // 通常のテクスチャ用サンプラー
SamplerComparisonState ShadowSampleType : register(s1); // シャドウマップ比較用サンプラー

cbuffer MaterialBuffer : register(b2)
{
    float4 EmissiveColor;
    bool UseTexture;
    float3 Padding;
};

// --- ライトの定義 ---
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

struct Light
{
    float4 Color;
    float3 Direction;
    float SpotAngle;
    float3 Position;
    float Range;
    float3 Attenuation;
    int Type;
    bool Enabled;
    float Intensity;
    float2 Padding;
};
// ライト情報を格納するコンスタントバッファ
cbuffer LightBuffer : register(b1)
{
    Light Lights[16];
    int NumLights;
    float3 CameraPosition;
};
// ピクセルシェーダーへの入力データ構造
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 Tex : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 WorldPos : WORLDPOS;
    float4 LightViewPos : TEXCOORD1;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
};

float4 PS(VS_OUTPUT input) : SV_Target
{
    // --- テクスチャ色の取得処理を修正 ---
    float4 textureColor;
    if (UseTexture)
    {
        // UseTextureがtrueならテクスチャから色を取得
        textureColor = shaderTexture.Sample(SampleType, input.Tex);
    }
    else
    {
        // falseなら白色を基本色とする
        textureColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    
    // --- ノーマルマッピング ---
    float3x3 tbnMatrix = float3x3(normalize(input.Tangent), normalize(input.Binormal), normalize(input.Normal));
    float3 normalMapSample = normalMapTexture.Sample(SampleType, input.Tex).rgb;
    float3 normalFromMap = (2.0f * normalMapSample) - 1.0f;
    float3 finalNormal = normalize(mul(normalFromMap, tbnMatrix));
    // 環境光
    float4 ambient = textureColor * float4(0.3f, 0.3f, 0.3f, 1.0f);
    // 拡散光と鏡面反射光の合計を初期化
    float4 totalDiffuse = float4(0, 0, 0, 0);
    float4 totalSpecular = float4(0, 0, 0, 0);
    // カメラへの視線ベクトル
    float3 viewDir = normalize(CameraPosition - input.WorldPos);
    // --- 影の計算 ---
    float shadowFactor = 1.0f;
    float2 projectTexCoord;
    projectTexCoord.x = input.LightViewPos.x / input.LightViewPos.w / 2.0f + 0.5f;
    projectTexCoord.y = -input.LightViewPos.y / input.LightViewPos.w / 2.0f + 0.5f;
    if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
    {
        shadowFactor = shadowMapTexture.SampleCmpLevelZero(ShadowSampleType, projectTexCoord, input.LightViewPos.z / input.LightViewPos.w);
    }

    // --- ライティングの計算 ---
    for (int i = 0; i < NumLights; ++i)
    {
        if (!Lights[i].Enabled)
            continue;
        float3 lightDir;
        float distance;

        if (Lights[i].Type == DIRECTIONAL_LIGHT)
        {
            lightDir = -normalize(Lights[i].Direction);
            distance = 0.0f; // ディレクショナルライトの距離は0とする
        }
        else
        {
            lightDir = normalize(Lights[i].Position - input.WorldPos);
            distance = length(Lights[i].Position - input.WorldPos);
        }
        
        float attenuation = 1.0f;
        if (Lights[i].Type != DIRECTIONAL_LIGHT)
        {
            // 従来の距離による減衰
            attenuation = 1.0f / (Lights[i].Attenuation.x + Lights[i].Attenuation.y * distance + Lights[i].Attenuation.z * (distance * distance));

            // Rangeに基づいてスムーズにフェードアウトさせる計算を追加
            // (1 - 距離/範囲)^2 を計算し、滑らかな減衰カーブを作る
            float falloff = pow(saturate(1.0 - distance / Lights[i].Range), 2);
            
            // 2つの減衰を掛け合わせる
            attenuation *= falloff;
        }
        
        
        // スポットライトの減衰
        float spotFactor = 1.0f;
        if (Lights[i].Type == SPOT_LIGHT)
        {
            float3 lightToPixelDir = -lightDir;
            float spotCos = dot(lightToPixelDir, normalize(Lights[i].Direction));
            float outerConeCos = Lights[i].SpotAngle;
            float innerConeCos = min(1.0f, outerConeCos + 0.15f);
            spotFactor = smoothstep(outerConeCos, innerConeCos, spotCos);
        }

        // 拡散光の計算 (各ライトごとに影を適用)
        float diffuseIntensity = saturate(dot(finalNormal, lightDir));
        totalDiffuse += Lights[i].Color * diffuseIntensity * Lights[i].Intensity * attenuation * spotFactor * shadowFactor;
        
        // 鏡面反射光の計算 (各ライトごとに影を適用)
        float3 halfwayDir = normalize(lightDir + viewDir);
        float specAngle = saturate(dot(finalNormal, halfwayDir));
        float specular = pow(specAngle, 32.0f);
        totalSpecular += float4(1.0f, 1.0f, 1.0f, 1.0f) * specular * Lights[i].Intensity * attenuation * spotFactor * shadowFactor;
    }

    // --- 最終的な色の合成処理を修正 ---
    // ライティング計算結果と環境光を合成
    float4 finalColor = (textureColor * (totalDiffuse + ambient)) + totalSpecular;
    
    // 自己発光色を加算
    finalColor += EmissiveColor;

    finalColor.a = 1.0f; // アルファ値は1で固定

    // --- Vignette Effect ---
    float2 screenCenter = float2(0.5f, 0.5f);
    float2 texCoord = input.Pos.xy / float2(1280, 720);
    float dist = distance(texCoord, screenCenter);
    float vignetteIntensity = 1.0f;
    float vignetteSmoothness = 0.1f;
    float vignette = 1.0f - smoothstep(vignetteSmoothness, 1.0f, dist * vignetteIntensity);
    finalColor.rgb *= vignette;
    finalColor.a = textureColor.a;

    return finalColor;
}