#pragma once
#include "Model.h"
#include "Player.h"
#include "LightManager.h"
#include "Audio.h"

// オーブの種類を定義
enum class OrbType {
	Normal,
	MinimapZoomOut,
	EnemyRadar,
	Goal
};

class Orb {
public:
	Orb();
	~Orb();

	bool Initialize(ID3D11Device* device, const DirectX::XMFLOAT3& position, int lightIndex, OrbType type = OrbType::Normal);
	void Shutdown();
	bool Update(float deltaTime, Player* player, LightManager* lightManager, DirectX::SoundEffect* collectSound);

	Model* GetModel();
	bool IsCollected() const;
	DirectX::XMFLOAT3 GetPosition() const;
	OrbType GetType() const; // オーブの種類を取得する関数

private:
	std::unique_ptr<Model> m_model;
	DirectX::XMFLOAT3 m_position;

	bool m_isCollected;
	int m_lightIndex;
	float m_animationTimer;
	OrbType m_type; // オーブの種類を保持するメンバ変数
};