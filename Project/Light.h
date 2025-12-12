#pragma once
#include <DirectXMath.h>

// ライトの最大数を定義
constexpr int MAX_LIGHTS = 64;

// ライトの種類
enum LightType {
	DirectionalLight = 0,
	PointLight = 1,
	SpotLight = 2
};

// シェーダーの定数バッファと一致させるためのライト構造体
struct Light {
	DirectX::XMFLOAT4   color;       
	DirectX::XMFLOAT3   direction;   
	float               spotAngle;   
	DirectX::XMFLOAT3   position;    
	float               range;       
	DirectX::XMFLOAT3   attenuation;  
	int                 type;        
	bool                enabled;    
	float               intensity;   
	DirectX::XMFLOAT2   padding;     
};