// テクスチャとサンプラーステート
Texture2D shaderTexture : register(t0); // オブジェクトのテクスチャ
Texture2D shadowMapTexture : register(t1); // シャドウマップ（深度テクスチャ）
Texture2D normalMapTexture : register(t2);
SamplerState SampleType : register(s0); // 通常のテクスチャ用サンプラー
SamplerComparisonState ShadowSampleType : register(s1); // シャドウマップ比較用サンプラー


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
    // テクスチャから基本色を取得
    float4 textureColor = shaderTexture.Sample(SampleType, input.Tex);

    // --- ノーマルマッピング ---
    // TBN行列を構築
    float3x3 tbnMatrix = float3x3(normalize(input.Tangent), normalize(input.Binormal), normalize(input.Normal));
    
    // ノーマルマップから法線をサンプリングし、[-1, 1]の範囲に変換
    float3 normalMapSample = normalMapTexture.Sample(SampleType, input.Tex).rgb;
    float3 normalFromMap = (2.0f * normalMapSample) - 1.0f;

    // ノーマルマップの法線を接線空間からワールド空間へ変換
    float3 finalNormal = normalize(mul(normalFromMap, tbnMatrix));

    // 環境光
    float4 ambient = textureColor * float4(0.3f, 0.3f, 0.3f, 1.0f);

    // 拡散光と鏡面反射光の合計を初期化
    float4 totalDiffuse = float4(0, 0, 0, 0);
    float4 totalSpecular = float4(0, 0, 0, 0);

    // カメラへの視線ベクトル
    float3 viewDir = normalize(CameraPosition - input.WorldPos);

    // --- 影の計算 ---
    float shadowFactor = 1.0; // 1.0 = 光が当たる, 0.0 = 影
    
    // ライト視点での座標をテクスチャ座標(0~1)に変換
    float2 projectTexCoord;
    projectTexCoord.x = input.LightViewPos.x / input.LightViewPos.w / 2.0f + 0.5f;
    projectTexCoord.y = -input.LightViewPos.y / input.LightViewPos.w / 2.0f + 0.5f;

    // テクスチャ座標が範囲内にある場合のみ影の計算を行う
    if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
    {
        // シャドウマップの深度値と現在のピクセルの深度を比較
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
            distance = 1.0f;
        }
        else
        {
            lightDir = normalize(Lights[i].Position - input.WorldPos);
            distance = length(Lights[i].Position - input.WorldPos);
        }

        // 距離による減衰
        float attenuation = 1.0f;
        if (Lights[i].Type != DIRECTIONAL_LIGHT)
        {
            if (distance > Lights[i].Range)
                continue;
            attenuation = 1.0f / (Lights[i].Attenuation.x + Lights[i].Attenuation.y * distance + Lights[i].Attenuation.z * (distance * distance));
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

        // 拡散光の計算
        float diffuseIntensity = saturate(dot(finalNormal, lightDir)); // <--- 変更
        totalDiffuse += Lights[i].Color * diffuseIntensity * Lights[i].Intensity * attenuation * spotFactor;
        
        // 鏡面反射光の計算 
        float3 halfwayDir = normalize(lightDir + viewDir);
        float specAngle = saturate(dot(finalNormal, halfwayDir)); // <--- 変更
        float specular = pow(specAngle, 32.0f);
        totalSpecular += float4(1.0f, 1.0f, 1.0f, 1.0f) * specular * Lights[i].Intensity * attenuation * spotFactor;
    }

    // 最終的な色の合成
    float4 finalColor = (textureColor * (totalDiffuse + ambient)) * shadowFactor + (totalSpecular * shadowFactor);
    finalColor.a = textureColor.a;

// --- Vignette Effect ---
    // 画面の中心からの距離を計算
    float2 screenCenter = float2(0.5f, 0.5f);
    float2 texCoord = input.Pos.xy / float2(1280, 720); // Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT
    float dist = distance(texCoord, screenCenter);

    // ビネットの強度と滑らかさを設定
    float vignetteIntensity = 1.0f; // 効果の強さ
    float vignetteSmoothness = 0.1f; // 効果の滑らかさ

    // ビネット係数を計算
    float vignette = 1.0f - smoothstep(vignetteSmoothness, 1.0f, dist * vignetteIntensity);

    // 最終的な色にビネット効果を適用
    finalColor.rgb *= vignette;

    finalColor.a = textureColor.a;

    return finalColor;
}