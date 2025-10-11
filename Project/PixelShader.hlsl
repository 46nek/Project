Texture2D shaderTexture : register(t0);
Texture2D shadowMapTexture : register(t1);
SamplerState SampleType : register(s0);
SamplerComparisonState ShadowSampleType : register(s1);

// ライトの種類
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

// 個々のライトの情報を保持する構造体
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

// ライトの情報を格納するコンスタントバッファ
cbuffer LightBuffer : register(b1)
{
    Light Lights[16];
    int NumLights;
    float3 CameraPosition;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 Tex : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 WorldPos : WORLDPOS;
    float4 LightViewPos : TEXCOORD1;
};

float4 PS(VS_OUTPUT input) : SV_Target
{
    // テクスチャからオブジェクトの基本色を取得
    float4 textureColor = shaderTexture.Sample(SampleType, input.Tex);
    // 環境光を計算 (テクスチャ色を反映)
    float4 ambient = textureColor * float4(0.3f, 0.3f, 0.3f, 1.0f);
    // これから計算する拡散光と鏡面反射光を初期化
    float4 totalDiffuse = float4(0, 0, 0, 0);
    float4 totalSpecular = float4(0, 0, 0, 0);
    // カメラへの視線ベクトルを計算
    float3 viewDir = normalize(CameraPosition - input.WorldPos);

    float shadowFactor = 1.0;
    // 影の影響係数 (1.0 = 光が当たる, 0.0 = 影)

    // 1. 光源視点での座標を正規化デバイス座標(-1~+1)からテクスチャ座標(0~1)に変換
    float2 projectTexCoord;
    projectTexCoord.x = input.LightViewPos.x / input.LightViewPos.w / 2.0f + 0.5f;
    projectTexCoord.y = -input.LightViewPos.y / input.LightViewPos.w / 2.0f + 0.5f;
    // 2. テクスチャ座標が0~1の範囲内にある場合のみ影の計算を行う
    if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
    {
        // 3. シャドウマップから深度値を取得し、現在のピクセルの深度と比較する
        // SampleCmpLevelZeroは、ハードウェアの機能を使って比較を行い、影なら0.0、そうでなければ1.0を返す
        shadowFactor = shadowMapTexture.SampleCmpLevelZero(ShadowSampleType, projectTexCoord, input.LightViewPos.z / input.LightViewPos.w);
    }

    // シーン内のすべてのライトをループ処理
    for (int i = 0; i < NumLights; ++i)
    {
        // ライトが無効ならスキップ
        if (!Lights[i].Enabled)
        {
            continue;
        }

        // --- ライトごとの基本情報を計算 ---
        float3 lightDir;
        float distance;

        if (Lights[i].Type == DIRECTIONAL_LIGHT)
        {
            lightDir = -normalize(Lights[i].Direction);
            distance = 1.0f; // 距離は無限
        }
        else
        {
            lightDir = normalize(Lights[i].Position - input.WorldPos);
            distance = length(Lights[i].Position - input.WorldPos);
        }

        // --- 距離による減衰を計算 ---
        float attenuation = 1.0f;
        if (Lights[i].Type != DIRECTIONAL_LIGHT)
        {
            if (distance > Lights[i].Range)
            {
                continue; // 範囲外ならスキップ
            }
            attenuation = 1.0f / (Lights[i].Attenuation.x + Lights[i].Attenuation.y * distance + Lights[i].Attenuation.z * (distance * distance));
        }
        
        float spotFactor = 1.0f;
        if (Lights[i].Type == SPOT_LIGHT)
        {
            float3 lightToPixelDir = -lightDir;
            float spotCos = dot(lightToPixelDir, normalize(Lights[i].Direction));

            // 外側のコーンの角度（コサイン）
            float outerConeCos = Lights[i].SpotAngle;
            // 内側のコーンの角度（コサイン）。光が完全に当たる部分。
            // コサイン値は角度が小さいほど大きくなるので、少し値を足します。
            float innerConeCos = min(1.0f, outerConeCos + 0.05f);

            // smoothstepを使って、内側と外側の間で滑らかに光の強さを変化させる
            // これにより、ピクセルがコーンの外側ならspotFactorは0になります。
            spotFactor = smoothstep(outerConeCos, innerConeCos, spotCos);
        }

        // --- 拡散光(Diffuse)を計算 ---
        float diffuseIntensity = saturate(dot(input.Normal, lightDir));
        totalDiffuse += Lights[i].Color * diffuseIntensity * Lights[i].Intensity * attenuation * spotFactor;
        // --- 鏡面反射光(Specular)を計算 ---
        float3 halfwayDir = normalize(lightDir + viewDir);
        float specAngle = saturate(dot(input.Normal, halfwayDir));
        float specular = pow(specAngle, 32.0f);
        // 光沢の強さ
        totalSpecular += float4(1.0f, 1.0f, 1.0f, 1.0f) * specular * Lights[i].Intensity * attenuation * spotFactor;
    }

    float4 finalColor = (textureColor * (totalDiffuse + ambient)) * shadowFactor + (totalSpecular * shadowFactor);
    finalColor.a = textureColor.a;

    return finalColor;
}