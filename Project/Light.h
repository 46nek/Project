#pragma once
#include <DirectXMath.h>

// ライトの最大数を定義
constexpr int MAX_LIGHTS = 64;

// ライトの種類
enum LightType
{
	DirectionalLight = 0,
	PointLight = 1,
	SpotLight = 2
};

// シェーダーの定数バッファと一致させるためのライト構造体
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
	DirectX::XMFLOAT2   Padding; // 16バイトアラインメントのためのパディング
};