#include "Orb.h"
#include "AssetLoader.h"
#include "AssetPaths.h"
#include <cmath>

Orb::Orb()
	: m_isCollected(false), 
	m_isDelivered(false),
	m_lightIndex(-1),
	m_animationTimer(0.0f),
	m_type(OrbType::Normal) {
}

Orb::~Orb() {
}

bool Orb::Initialize(ID3D11Device* device, const DirectX::XMFLOAT3& position, int lightIndex, OrbType type) {
	m_position = position;
	m_lightIndex = lightIndex;
	m_type = type;

	m_model = AssetLoader::LoadModelFromFile(device, AssetPaths::MODEL_CUBE_OBJ);
	if (!m_model) {
		return false;
	}
	m_model->SetScale(0.3f, 0.3f, 0.3f);
	m_model->SetPosition(m_position.x, m_position.y, m_position.z);

	// オーブの種類に応じて色を変える
	switch (m_type) {
	case OrbType::MinimapZoomOut:
		m_model->SetEmissiveColor({ 0.2f, 1.0f, 0.2f, 1.0f }); // 緑
		break;
	case OrbType::EnemyRadar:
		m_model->SetEmissiveColor({ 1.0f, 0.2f, 0.2f, 1.0f }); // 赤
		break;
	case OrbType::Goal:
		m_model->SetEmissiveColor({ 1.0f, 0.8f, 0.0f, 1.0f });
		break;
	case OrbType::Normal:
	default:
		m_model->SetEmissiveColor({ 0.6f, 0.8f, 1.0f, 1.0f }); // 青
		break;
	}

	m_model->SetUseTexture(false);

	m_animationTimer = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 10.0f;

	return true;
}

void Orb::Shutdown() {
	if (m_model) {
		m_model->Shutdown();
	}
}

bool Orb::Update(float deltaTime, Player* player, LightManager* lightManager, DirectX::SoundEffect* collectSound) {
	if (m_isCollected) {
		return false; // すでに収集済みなら false を返す
	}

	float floatingY = m_position.y + sin(m_animationTimer * 2.0f) * 0.25f;
	m_animationTimer += deltaTime;
	m_model->SetPosition(m_position.x, floatingY, m_position.z);

	DirectX::XMFLOAT3 playerPos = player->GetPosition();
	float dx = playerPos.x - m_position.x;
	float dz = playerPos.z - m_position.z;
	float distanceSq = (dx * dx) + (dz * dz);

	float collisionRadius = 1.0f;
	float collisionRadiusSq = collisionRadius * collisionRadius;

	if (distanceSq < collisionRadiusSq) {
		m_isCollected = true;
		return true;
	}

	return false; // 収集されなかった場合は false を返す
}

void Orb::FollowPlayer(float deltaTime, const DirectX::XMFLOAT3& targetPos, int index) {
	if (m_isDelivered) return;

	float angle = m_animationTimer * 1.5f + static_cast<float>(index);
	DirectX::XMFLOAT3 followOffset = {
		sinf(angle) * 0.7f,
		0.5f + sinf(m_animationTimer * 2.0f) * 0.1f, // ゆらゆら
		cosf(angle) * 0.7f
	};

	m_position.x += (targetPos.x + followOffset.x - m_position.x) * deltaTime * 3.0f;
	m_position.y += (targetPos.y + followOffset.y - m_position.y) * deltaTime * 3.0f;
	m_position.z += (targetPos.z + followOffset.z - m_position.z) * deltaTime * 3.0f;

	m_model->SetPosition(m_position.x, m_position.y, m_position.z);
	m_animationTimer += deltaTime;
}

Model* Orb::GetModel() {
	return m_isDelivered ? nullptr : m_model.get();
}

bool Orb::IsCollected() const {
	return m_isCollected;
}

DirectX::XMFLOAT3 Orb::GetPosition() const {
	return m_position;
}

OrbType Orb::GetType() const {
	return m_type;
}