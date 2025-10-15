// Orb.h (この内容で完全に置き換えてください)

#pragma once
#include "Model.h"
#include "Player.h"
#include "LightManager.h"

/**
 * @brief 回収可能なオーブを管理するクラス
 */
class Orb
{
public:
	Orb();
	~Orb();

	bool Initialize(ID3D11Device* device, const DirectX::XMFLOAT3& position, int lightIndex);
	void Shutdown();
	void Update(float deltaTime, Player* player, LightManager* lightManager);
	Model* GetModel();
	bool IsCollected() const;
	DirectX::XMFLOAT3 GetPosition() const; // <--- この行を追加

private:
	std::unique_ptr<Model> m_model;
	DirectX::XMFLOAT3 m_position;
	bool m_isCollected;
	int m_lightIndex;
	float m_animationTimer;
};