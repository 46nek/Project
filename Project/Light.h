#pragma once
#include <DirectXMath.h>

// ���C�g�̎�ނ��`
enum LightType
{
    DirectionalLight = 0,
    PointLight = 1,
    SpotLight = 2
};

// �X�̃��C�g�̏���ێ�����\����
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