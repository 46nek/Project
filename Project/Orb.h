// Orb.h (���̓��e�Ŋ��S�ɒu�������Ă�������)

#pragma once
#include "Model.h"
#include "Player.h"
#include "LightManager.h"
#include "Audio.h"

// �I�[�u�̎�ނ��`
enum class OrbType
{
	Normal,
	MinimapZoomOut,
	EnemyRadar
};

class Orb
{
public:
	Orb();
	~Orb();

	bool Initialize(ID3D11Device* device, const DirectX::XMFLOAT3& position, int lightIndex, OrbType type = OrbType::Normal);
	void Shutdown();
	void Update(float deltaTime, Player* player, LightManager* lightManager, DirectX::SoundEffect* collectSound);

	Model* GetModel();
	bool IsCollected() const;
	DirectX::XMFLOAT3 GetPosition() const;
	OrbType GetType() const; // �I�[�u�̎�ނ��擾����֐�

private:
	std::unique_ptr<Model> m_model;
	DirectX::XMFLOAT3 m_position;

	bool m_isCollected;
	int m_lightIndex;
	float m_animationTimer;
	OrbType m_type; // �I�[�u�̎�ނ�ێ����郁���o�ϐ�
};