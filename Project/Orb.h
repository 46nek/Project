// Orb.h (���̓��e�Ŋ��S�ɒu�������Ă�������)

#pragma once
#include "Model.h"
#include "Player.h"
#include "LightManager.h"
#include "Audio.h"

class Orb
{
public:
	Orb();
	~Orb();

	bool Initialize(ID3D11Device* device, const DirectX::XMFLOAT3& position, int lightIndex);
	void Shutdown();
	void Update(float deltaTime, Player* player, LightManager* lightManager, DirectX::SoundEffect* collectSound);

	Model* GetModel();
	bool IsCollected() const; // <--- GetState() ���� IsCollected() �ɖ߂��܂�
	DirectX::XMFLOAT3 GetPosition() const;

private:
	std::unique_ptr<Model> m_model;
	DirectX::XMFLOAT3 m_position;

	bool m_isCollected;      // <--- enum OrbState ���� bool �ɖ߂��܂�
	int m_lightIndex;
	float m_animationTimer;
};