#pragma once
#include <DirectXMath.h>

// ���C�g�̍ő吔���`
constexpr int MAX_LIGHTS = 64;

// ���C�g�̎��
enum LightType
{
	DirectionalLight = 0,
	PointLight = 1,
	SpotLight = 2
};

// �V�F�[�_�[�̒萔�o�b�t�@�ƈ�v�����邽�߂̃��C�g�\����
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
	DirectX::XMFLOAT2   Padding; // 16�o�C�g�A���C�������g�̂��߂̃p�f�B���O
};