#pragma once
#include <DirectXMath.h>

// ライトの種類を定義
enum LightType
{
    DirectionalLight = 0,
    PointLight = 1,
    SpotLight = 2
};

// 個々のライトの情報を保持する構造体
struct Light
{
    DirectX::XMFLOAT4   Color;
    DirectX::XMFLOAT3   Direction;
    float               SpotAngle;
    DirectX::XMFLOAT3   Position;
    float               Range;
    DirectX::XMFLOAT3   Attenuation;
    int                 Type;
    bool                Enabled;
    float               Intensity;
    DirectX::XMFLOAT2   Padding;
};