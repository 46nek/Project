#include "Enemy.h"
#include "AssetLoader.h"
#include <random>
#include <cmath>

// コンストラクタ
Enemy::Enemy()
	: m_speed(4.0f),
	m_pathIndex(-1),
	m_pathCooldown(0.0f)
{
}

Enemy::~Enemy() {}

// 初期化
bool Enemy::Initialize(ID3D11Device* device, const DirectX::XMFLOAT3& startPosition, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData)
{
	m_position = startPosition;
	m_model = AssetLoader::LoadModelFromFile(device, "Assets/cube.obj");
	if (!m_model) return false;

	m_model->SetPosition(startPosition.x, startPosition.y, startPosition.z);

	// A*探索用のインスタンスを作成
	m_astar = std::make_unique<AStar>(mazeData);

	return true;
}

void Enemy::Shutdown()
{
	if (m_model) m_model->Shutdown();
}

// 毎フレームの更新処理
void Enemy::Update(float deltaTime, const Player* player, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth)
{
	if (!player || !m_model) return;

	// プレイヤーとの距離を計算
	DirectX::XMFLOAT3 playerPos = player->GetPosition();
	float dx = playerPos.x - m_position.x;
	float dz = playerPos.z - m_position.z;
	float distanceSq = (dx * dx) + (dz * dz); // 距離の2乗で比較

	// === 状態ごとの行動 ===
	m_pathCooldown -= deltaTime;
	if (m_pathCooldown <= 0.0f)
	{
		int startX = static_cast<int>(m_position.x / pathWidth);
		int startY = static_cast<int>(m_position.z / pathWidth);
		int goalX = static_cast<int>(playerPos.x / pathWidth);
		int goalY = static_cast<int>(playerPos.z / pathWidth);

		m_pathCooldown = 2.0f;

		// --- 結果を一時変数で受け取る ---
		std::vector<DirectX::XMFLOAT2> newPath = m_astar->FindPath(startX, startY, goalX, goalY);

		// パスが見つかった場合のみ更新する！
		// (失敗しても、今のパス情報を捨てずに維持する)
		if (!newPath.empty())
		{
			m_path = newPath;
			// 新しいパスの 1番目 (0番目は現在地なので次は1番目) を目指す
			// ただし、パスが "現在地のみ" の1個しかない場合はゴール済みとする
			m_pathIndex = (m_path.size() > 1) ? 1 : -1;
		}
	}

	// === パスに沿った移動処理 (共通) ===
	if (m_pathIndex != -1 && m_pathIndex < m_path.size())
	{
		DirectX::XMFLOAT2 nextGridPos = m_path[m_pathIndex];
		DirectX::XMFLOAT3 targetPosition = {
			(nextGridPos.x + 0.5f) * pathWidth,
			m_position.y,
			(nextGridPos.y + 0.5f) * pathWidth
		};

		DirectX::XMVECTOR vecToTarget = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&targetPosition), DirectX::XMLoadFloat3(&m_position));
		float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(vecToTarget));

		if (distance < 0.1f)
		{
			m_pathIndex++;
		}
		else
		{
			DirectX::XMVECTOR moveVec = DirectX::XMVector3Normalize(vecToTarget);
			moveVec = DirectX::XMVectorScale(moveVec, m_speed * deltaTime);
			DirectX::XMStoreFloat3(&m_position, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&m_position), moveVec));
			m_model->SetPosition(m_position.x, m_position.y, m_position.z);
		}
	}
}

Model* Enemy::GetModel()
{
	return m_model.get();
}

DirectX::XMFLOAT3 Enemy::GetPosition() const
{
	return m_position;
}