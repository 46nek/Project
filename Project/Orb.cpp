#include "Orb.h"
#include "AssetLoader.h"
#include "AssetPaths.h"
#include <cmath>
#include <random>

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

	// 繧ｪ繝ｼ繝悶・遞ｮ鬘槭↓蠢懊§縺ｦ濶ｲ繧貞､峨∴繧・
	switch (m_type) {
	case OrbType::MinimapZoomOut:
		m_model->SetEmissiveColor({ 0.2f, 1.0f, 0.2f, 1.0f }); // 邱・
		break;
	case OrbType::EnemyRadar:
		m_model->SetEmissiveColor({ 1.0f, 0.2f, 0.2f, 1.0f }); // 襍､
		break;
	case OrbType::Goal:
		m_model->SetEmissiveColor({ 1.0f, 0.8f, 0.0f, 1.0f }); // 繧ｴ繝ｼ繝ｫ繝・
		break;
	case OrbType::Normal:
	default:
		m_model->SetEmissiveColor({ 0.6f, 0.8f, 1.0f, 1.0f }); // 髱・
		break;
	}

	m_model->SetUseTexture(false);

	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(0.0f, 10.0f);
	m_animationTimer = dist(gen);

	return true;
}

void Orb::Shutdown() {
	if (m_model) {
		m_model->Shutdown();
	}
}

bool Orb::Update(float deltaTime, Player* player, LightManager* lightManager, DirectX::SoundEffect* collectSound) {
	if (m_isCollected) {
		return false; // 縺吶〒縺ｫ蜿朱寔貂医∩縺ｪ繧・false 繧定ｿ斐☆
	}

	float floatingY = m_position.y + std::sin(m_animationTimer * 2.0f) * 0.25f;
	m_animationTimer += deltaTime;
	m_model->SetPosition(m_position.x, floatingY, m_position.z);

	DirectX::XMFLOAT3 playerPos = player->GetPosition();
	float dx = playerPos.x - m_position.x;
	float dz = playerPos.z - m_position.z;
	float distanceSq = (dx * dx) + (dz * dz);

	float collisionRadius = 1.0f;
	float collisionRadiusSq = collisionRadius * collisionRadius;

	if (distanceSq < collisionRadiusSq) {
		if (collectSound) {
			collectSound->Play();
		}
		m_isCollected = true;
		return true;
	}

	return false; // 蜿朱寔縺輔ｌ縺ｪ縺九▲縺溷ｴ蜷医・ false 繧定ｿ斐☆
}

void Orb::FollowPlayer(float deltaTime, const DirectX::XMFLOAT3& targetPos, int index) {
	if (m_isDelivered) return;

	float verticalOffset = (index == 0) ? -1.5f : 0.0f;

	float sway = std::sin(m_animationTimer * 2.5f) * 0.2f;

	m_position.x += (targetPos.x - m_position.x) * deltaTime * 3.0f;
	m_position.y += (targetPos.y + verticalOffset + sway - m_position.y) * deltaTime * 3.0f;
	m_position.z += (targetPos.z - m_position.z) * deltaTime * 3.0f;

	m_model->SetPosition(m_position.x, m_position.y, m_position.z);
	m_animationTimer += deltaTime;
}

Model* Orb::GetModel() {
	if (m_isDelivered) return nullptr;

	if (m_type != OrbType::Normal && m_isCollected) {
		return nullptr;
	}

	return m_model.get();
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
