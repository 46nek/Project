#include "Enemy.h"
#include "GraphicsDevice.h"
#include <random>
#include <cmath>

namespace {
	constexpr float PATH_COOLDOWN_TIME = 0.4f;
}

Enemy::Enemy()
	: m_speed(6.0f), m_pathIndex(-1), m_pathCooldown(0.0f) {
}

Enemy::~Enemy() {
}

bool Enemy::Initialize(ID3D11Device* device, const DirectX::XMFLOAT3& startPosition, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData) {
	m_position = startPosition;

	m_particleSystem = std::make_unique<ParticleSystem>();
	if (!m_particleSystem->Initialize(device)) {
		return false;
	}

	m_astar = std::make_unique<AStar>(mazeData);
	return true;
}

void Enemy::Shutdown() {
	if (m_particleSystem) { m_particleSystem->Shutdown(); }
}

void Enemy::Update(float deltaTime, const Player* player, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth) {
	if (!player || !m_particleSystem) { return; }

	DirectX::XMFLOAT3 playerPos = player->GetPosition();
	m_pathCooldown -= deltaTime;

	if (m_pathCooldown <= 0.0f) {
		int startX = static_cast<int>(m_position.x / pathWidth);
		int startY = static_cast<int>(m_position.z / pathWidth);
		int goalX = static_cast<int>(playerPos.x / pathWidth);
		int goalY = static_cast<int>(playerPos.z / pathWidth);

		m_pathCooldown = PATH_COOLDOWN_TIME;
		std::vector<DirectX::XMFLOAT2> newPath = m_astar->FindPath(startX, startY, goalX, goalY);

		if (!newPath.empty()) {
			m_path = newPath;
			m_pathIndex = (m_path.size() > 1) ? 1 : -1;
		}
	}

	if (m_pathIndex != -1 && m_pathIndex < static_cast<int>(m_path.size())) {
		DirectX::XMFLOAT2 nextGridPos = m_path[m_pathIndex];
		DirectX::XMFLOAT3 targetPosition = {
			(nextGridPos.x + 0.5f) * pathWidth,
			m_position.y,
			(nextGridPos.y + 0.5f) * pathWidth
		};

		DirectX::XMVECTOR vecToTarget = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&targetPosition), DirectX::XMLoadFloat3(&m_position));
		float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(vecToTarget));

		if (distance < 0.1f) {
			m_pathIndex++;
		}
		else {
			DirectX::XMVECTOR moveVec = DirectX::XMVector3Normalize(vecToTarget);
			moveVec = DirectX::XMVectorScale(moveVec, m_speed * deltaTime);
			DirectX::XMStoreFloat3(&m_position, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&m_position), moveVec));
		}
	}

	// --- パーティクルの更新 ---
	m_particleSystem->Update(deltaTime, m_position);
}

// 描画: パーティクルシステムに委譲
void Enemy::Render(GraphicsDevice* graphicsDevice,
	const DirectX::XMMATRIX& viewMatrix,
	const DirectX::XMMATRIX& projectionMatrix,
	const DirectX::XMMATRIX& lightViewMatrix,
	const DirectX::XMMATRIX& lightProjectionMatrix) {

	if (m_particleSystem) {
		m_particleSystem->Render(graphicsDevice, viewMatrix, projectionMatrix, lightViewMatrix, lightProjectionMatrix);
	}
}

DirectX::XMFLOAT3 Enemy::GetPosition() const {
	return m_position;
}