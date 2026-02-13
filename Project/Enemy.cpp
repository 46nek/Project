#include "Enemy.h"
#include "GraphicsDevice.h"
#include "Decoy.h"
#include "Stage.h"
#include <random>
#include <cmath>

namespace {
	constexpr float PATH_COOLDOWN_TIME = 0.5f; // パス計算の間隔
	constexpr float SEARCH_DURATION = 3.0f;    // 探索時間
	constexpr float AMBUSH_DURATION = 5.0f;    // 待ち伏せ最大時間
	constexpr float SIGHT_RANGE = 20.0f;       // 視認距離
	constexpr float HEAR_RANGE_NORMAL = 5.0f;  // 通常聴覚範囲
	constexpr float HEAR_RANGE_RUN = 30.0f;    // ダッシュ時の聴覚範囲
}

Enemy::Enemy()
	: m_speed(6.0f), m_pathIndex(-1), m_pathCooldown(0.0f),
	m_currentState(PATROL), m_searchTimer(0.0f), m_ambushTimer(0.0f),
	m_lastKnownPlayerPos(0.0f, 0.0f, 0.0f), m_ambushTarget(0.0f, 0.0f, 0.0f),
	m_prevPosition(0.0f, 0.0f, 0.0f), m_stuckTimer(0.0f) {
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

void Enemy::Update(float deltaTime, const Player* player, const Stage* stage, const std::vector<Decoy*>& decoys, const std::vector<std::unique_ptr<Enemy>>& enemies) {
	if (!player || !stage) return;

	const auto& mazeData = stage->GetMazeData();
	float pathWidth = stage->GetPathWidth();

	DirectX::XMFLOAT3 playerPos = player->GetPosition();
	float distToPlayerSq = (playerPos.x - m_position.x) * (playerPos.x - m_position.x) + (playerPos.z - m_position.z) * (playerPos.z - m_position.z);
	float distToPlayer = std::sqrt(distToPlayerSq);

	// --- 感覚処理 ---
	bool canSee = CanSeePlayer(playerPos, mazeData, pathWidth);
	bool canHear = (player->IsSkillActive() && distToPlayer < HEAR_RANGE_RUN) || (distToPlayer < HEAR_RANGE_NORMAL);

	// デコイ優先処理
	DirectX::XMFLOAT3 targetPos = playerPos;
	bool chasingDecoy = false;
	float minDecoyDistSq = FLT_MAX;
	for (auto decoy : decoys) {
		float dx = decoy->GetPosition().x - m_position.x;
		float dz = decoy->GetPosition().z - m_position.z;
		float dSq = dx * dx + dz * dz;
		if (dSq < 225.0f) { // デコイ反応範囲
			if (dSq < minDecoyDistSq) {
				minDecoyDistSq = dSq;
				targetPos = decoy->GetPosition();
				chasingDecoy = true;
			}
		}
	}

	if (chasingDecoy) {
		m_currentState = CHASE; // デコイを追う時は無条件で追跡
	}
	else {
		// --- ステート遷移 ---
		switch (m_currentState) {
		case PATROL:
			if (canSee || canHear) {
				m_currentState = CHASE;
			}
			else {
				// 目的地（m_ambushTargetを流用）がない、または到着したら次の目的地を設定
				float dx = m_ambushTarget.x - m_position.x;
				float dz = m_ambushTarget.z - m_position.z;
				if (dx * dx + dz * dz < 1.0f || m_path.empty()) {
					const auto& rooms = stage->GetRooms();
					if (!rooms.empty()) {
						int idx = static_cast<int>(rand() % rooms.size());
						m_ambushTarget = rooms[idx].center;
						m_ambushTarget.x = m_ambushTarget.x * pathWidth;
						m_ambushTarget.z = m_ambushTarget.z * pathWidth;
					}
				}
				targetPos = m_ambushTarget;
			}
			break;

		case CHASE:
			if (canSee || canHear) {
				m_lastKnownPlayerPos = playerPos;
			}
			else {
				// 見失った -> 探索へ
				m_currentState = SEARCH;
				m_searchTimer = SEARCH_DURATION;
				targetPos = m_lastKnownPlayerPos; // 最後に見た場所へ行く
			}
			break;

		case SEARCH:
			if (canSee || canHear) {
				m_currentState = CHASE;
			}
			else {
				m_searchTimer -= deltaTime;
				if (m_searchTimer <= 0.0f) {
					// 探索終了 -> 待ち伏せ または 巡回
					// ここではランダムに部屋を選んで待ち伏せする
					m_currentState = AMBUSH;
					m_ambushTimer = AMBUSH_DURATION;
					
					const auto& rooms = stage->GetRooms();
					if (!rooms.empty()) {
						int idx = static_cast<int>(rand() % rooms.size());
						m_ambushTarget = rooms[idx].center;
						m_ambushTarget.x = m_ambushTarget.x * pathWidth;
						m_ambushTarget.z = m_ambushTarget.z * pathWidth;
					} else {
						m_currentState = PATROL; // 部屋がなければPATROL
					}
				}
				targetPos = m_lastKnownPlayerPos; // 探索中は最後の位置へ向かう(着いたらウロウロさせたいが今回は位置維持)
			}
			break;

		case AMBUSH:
			if (canSee || canHear) {
				m_currentState = CHASE;
			}
			else {
				targetPos = m_ambushTarget;
				m_ambushTimer -= deltaTime;
				if (m_ambushTimer <= 0.0f) {
					m_currentState = PATROL;
				}
			}
			break;
		}
	}

	// MOVE LOGIC (A*)
	// ターゲットが変わらない限り、パス再計算はクールダウンごとに行う
	m_pathCooldown -= deltaTime;
	if (true) { // 常に移動判定を行う (PATROL, SEARCH, AMBUSH, CHASE 全てで移動可能性があるため)
		if (m_pathCooldown <= 0.0f) {
			m_pathCooldown = PATH_COOLDOWN_TIME;

			int startX = static_cast<int>(m_position.x / pathWidth);
			int startY = static_cast<int>(m_position.z / pathWidth);
			int goalX = static_cast<int>(targetPos.x / pathWidth);
			int goalY = static_cast<int>(targetPos.z / pathWidth);

			// マップ範囲外チェック
			int w = mazeData[0].size();
			int h = mazeData.size();
			if (goalX >= 0 && goalX < w && goalY >= 0 && goalY < h) {
				m_path = m_astar->FindPath(startX, startY, goalX, goalY);
				m_pathIndex = (m_path.size() > 1) ? 1 : -1;
			}
		}

		if (m_pathIndex != -1 && m_pathIndex < static_cast<int>(m_path.size())) {
			DirectX::XMFLOAT2 nextGridPos = m_path[m_pathIndex];
			DirectX::XMFLOAT3 targetPosition(
				(nextGridPos.x + 0.5f) * pathWidth,
				m_position.y,
				(nextGridPos.y + 0.5f) * pathWidth
			);

			DirectX::XMVECTOR vecToTarget = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&targetPosition), DirectX::XMLoadFloat3(&m_position));
			float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(vecToTarget));

			if (distance < 0.1f) {
				m_pathIndex++;
			}
			else {
				float speed = m_speed;
				if (m_currentState == AMBUSH) speed *= 1.5f; // 先回り時は速く移動
				if (m_currentState == CHASE && canSee) speed *= 1.2f; // 視認中は加速

				DirectX::XMVECTOR moveVec = DirectX::XMVector3Normalize(vecToTarget);
				
				// --- 分離（Separation）挙動 ---
				DirectX::XMVECTOR separationVec = DirectX::XMVectorZero();
				int neighborCount = 0;
				for (const auto& otherEnemy : enemies) {
					if (otherEnemy.get() == this) continue;

					DirectX::XMFLOAT3 neighborPos = otherEnemy->GetPosition();
					DirectX::XMVECTOR otherPos = DirectX::XMLoadFloat3(&neighborPos);
					DirectX::XMVECTOR toNeighbor = DirectX::XMVectorSubtract(otherPos, DirectX::XMLoadFloat3(&m_position));
					float distToNeighbor = DirectX::XMVectorGetX(DirectX::XMVector3Length(toNeighbor));

					if (distToNeighbor < 2.0f && distToNeighbor > 0.001f) { // 2.0f以内の敵から離れる
						DirectX::XMVECTOR flee = DirectX::XMVectorScale(DirectX::XMVector3Normalize(toNeighbor), -1.0f);
						flee = DirectX::XMVectorScale(flee, 1.0f / distToNeighbor); // 近いほど強く離れる
						separationVec = DirectX::XMVectorAdd(separationVec, flee);
						neighborCount++;
					}
				}

				if (neighborCount > 0) {
					separationVec = DirectX::XMVectorScale(separationVec, 1.5f); // 分離の重み
					moveVec = DirectX::XMVectorAdd(moveVec, separationVec);
					moveVec = DirectX::XMVector3Normalize(moveVec); // 正規化しなおす

					// 同じ場所に留まり続けるのを防ぐため、混雑時は確率でターゲット変更（巡回時に限る）
					if (m_currentState == PATROL && (rand() % 100) < 2) { // 2%の確率で変更
				 		m_path.clear(); // 強制的にパスリセット
						m_pathIndex = -1;
					}
				}

				moveVec = DirectX::XMVectorScale(moveVec, speed * deltaTime);
				DirectX::XMStoreFloat3(&m_position, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&m_position), moveVec));

				// --- スタック検知 ---
				float movedDistSq = 
					(m_position.x - m_prevPosition.x) * (m_position.x - m_prevPosition.x) +
					(m_position.z - m_prevPosition.z) * (m_position.z - m_prevPosition.z);
				
				if (movedDistSq < 0.0001f) { // ほとんど動いていない
					m_stuckTimer += deltaTime;
					if (m_stuckTimer > 2.0f) { // 2秒以上スタック
						// スタック脱出：ランダムな方向へ少し動かすか、ターゲットを強制変更
						m_path.clear();
						m_pathIndex = -1;
						m_ambushTarget = m_position; // 一旦ターゲットを自分にしてリセット
						m_currentState = PATROL; // 巡回に戻す
						m_stuckTimer = 0.0f;
					}
				} else {
					m_stuckTimer = 0.0f; // 動いたらリセット
				}
				m_prevPosition = m_position;
			}
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

bool Enemy::CanSeePlayer(const DirectX::XMFLOAT3& playerPos, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth) {
	DirectX::XMVECTOR vP = DirectX::XMLoadFloat3(&m_position);
	DirectX::XMVECTOR vT = DirectX::XMLoadFloat3(&playerPos);
	DirectX::XMVECTOR vDir = DirectX::XMVectorSubtract(vT, vP);
	float dist = DirectX::XMVectorGetX(DirectX::XMVector3Length(vDir));

	if (dist > SIGHT_RANGE) return false;

	// レイキャスト (簡易版: ステップサイズごとに壁判定)
	vDir = DirectX::XMVector3Normalize(vDir);
	float step = pathWidth * 0.5f;
	int steps = static_cast<int>(dist / step);
	
	DirectX::XMFLOAT3 checkPos = m_position;
	DirectX::XMVECTOR vCheck = vP;

	for (int i = 0; i < steps; ++i) {
		vCheck = DirectX::XMVectorAdd(vCheck, DirectX::XMVectorScale(vDir, step));
		DirectX::XMStoreFloat3(&checkPos, vCheck);

		int x = static_cast<int>(checkPos.x / pathWidth);
		int y = static_cast<int>(checkPos.z / pathWidth);

		if (x >= 0 && x < mazeData[0].size() && y >= 0 && y < mazeData.size()) {
			if (mazeData[y][x] == MazeGenerator::Wall) {
				return false; // 壁に遮られた
			}
		}
	}
	return true;
}