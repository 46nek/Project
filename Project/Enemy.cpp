#include "Enemy.h"
#include "AssetLoader.h"
#include <random>

// コンストラクタ
Enemy::Enemy()
	: m_speed(2.0f),
	m_pathIndex(-1),
	m_pathCooldown(0.0f),
	m_currentState(AIState::Wandering),
	m_stateTimer(0.0f)
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

// プレイヤーが見えるかどうかの判定（簡易版）
bool Enemy::CanSeePlayer(const Player* player, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth)
{
	if (!player) return false;

	DirectX::XMFLOAT3 playerPos = player->GetPosition();
	DirectX::XMVECTOR vecToPlayer = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&playerPos), DirectX::XMLoadFloat3(&m_position));

	// プレイヤーとの距離を計算
	float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(vecToPlayer));

	// 距離が15.0fより遠ければ「見えない」
	if (distance > 15.0f)
	{
		return false;
	}

	// 現状は距離だけで判定（今後のステップで視界や壁の判定を追加）
	return true;
}

// 徘徊モードの処理
void Enemy::Wander(float deltaTime, float pathWidth)
{
	m_stateTimer -= deltaTime;
	// 目標地点に到着したか、タイマーが切れたら新しい目標地点を設定
	if (m_pathIndex == -1 || m_stateTimer <= 0.0f)
	{
		m_stateTimer = 5.0f; // 5秒ごとに新しい場所へ

		int currentX = static_cast<int>(m_position.x / pathWidth);
		int currentY = static_cast<int>(m_position.z / pathWidth);

		int mazeWidth = m_astar->GetMazeWidth();
		int mazeHeight = m_astar->GetMazeHeight();
		int targetX, targetY;

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> distribX(0, mazeWidth - 1);
		std::uniform_int_distribution<> distribY(0, mazeHeight - 1);

		// 現在地から一定距離（マンハッタン距離で5以上）離れた有効な場所が見つかるまで探し続ける
		do {
			targetX = distribX(gen);
			targetY = distribY(gen);
		} while (abs(currentX - targetX) + abs(currentY - targetY) < 5 || !m_astar->IsWalkable(targetX, targetY));

		// 経路探索
		m_path = m_astar->FindPath(currentX, currentY, targetX, targetY);
		m_pathIndex = m_path.empty() ? -1 : 1;
	}
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

	// === 状態遷移の判定 ===
	switch (m_currentState)
	{
	case AIState::Wandering:
		m_speed = 4.0f;
		if (CanSeePlayer(player, mazeData, pathWidth))
		{
			m_currentState = AIState::Chasing;
			m_pathCooldown = 0.0f;
		}
		break;

	case AIState::Chasing:
		m_speed = 6.0f;
		if (!CanSeePlayer(player, mazeData, pathWidth))
		{
			m_stateTimer += deltaTime;
			if (m_stateTimer > 3.0f)
			{
				m_currentState = AIState::Wandering;
				m_pathIndex = -1;
			}
		}
		else
		{
			m_stateTimer = 0.0f;
		}
		break;
	}

	// === 状態ごとの行動 ===
	m_pathCooldown -= deltaTime;
	if (m_pathCooldown <= 0.0f)
	{
		switch (m_currentState)
		{
		case AIState::Wandering:
			// 徘徊モードでは5秒ごとに新しい目標を設定
			m_pathCooldown = 5.0f;
			Wander(deltaTime, pathWidth); // Wander関数は目標設定のみに利用
			break;

		case AIState::Chasing:
			// --- プレイヤーとの距離に応じて経路探索の頻度を変更 ---
			if (distanceSq > 15.0f * 15.0f) // 距離が15以上なら
			{
				m_pathCooldown = 3.0f; // 2秒ごと
			}
			else if (distanceSq > 10.0f * 10.0f) // 距離が10以上なら
			{
				m_pathCooldown = 1.5f; // 1秒ごと
			}
			else // 距離が10未満なら
			{
				m_pathCooldown = 0.8f; // 0.5秒ごと（より正確に追跡）
			}

			int startX = static_cast<int>(m_position.x / pathWidth);
			int startY = static_cast<int>(m_position.z / pathWidth);
			int goalX = static_cast<int>(playerPos.x / pathWidth);
			int goalY = static_cast<int>(playerPos.z / pathWidth);
			m_path = m_astar->FindPath(startX, startY, goalX, goalY);
			m_pathIndex = m_path.empty() ? -1 : 1;
			break;
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