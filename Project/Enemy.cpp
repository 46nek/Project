#include "Enemy.h"
#include "GraphicsDevice.h"
#include "Decoy.h"
#include "Stage.h"
#include <random>
#include <cmath>

namespace {
	constexpr float PATH_COOLDOWN_TIME = 0.5f; // 繝代せ險育ｮ励・髢馴囈
	constexpr float SEARCH_DURATION = 3.0f;    // 謗｢邏｢譎る俣
	constexpr float AMBUSH_DURATION = 5.0f;    // 蠕・■莨上○譛螟ｧ譎る俣
	constexpr float SIGHT_RANGE = 20.0f;       // 隕冶ｪ崎ｷ晞屬
	constexpr float HEAR_RANGE_NORMAL = 5.0f;  // 騾壼ｸｸ閨ｴ隕夂ｯ・峇
	constexpr float HEAR_RANGE_RUN = 30.0f;    // 繝繝・す繝･譎ゅ・閨ｴ隕夂ｯ・峇
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

	// --- 諢溯ｦ壼・逅・---
	bool canSee = CanSeePlayer(playerPos, mazeData, pathWidth);
	bool canHear = (player->IsSkillActive() && distToPlayer < HEAR_RANGE_RUN) || (distToPlayer < HEAR_RANGE_NORMAL);

	// 繝・さ繧､蜆ｪ蜈亥・逅・
	DirectX::XMFLOAT3 targetPos = playerPos;
	bool chasingDecoy = false;
	float minDecoyDistSq = FLT_MAX;
	for (auto decoy : decoys) {
		float dx = decoy->GetPosition().x - m_position.x;
		float dz = decoy->GetPosition().z - m_position.z;
		float dSq = dx * dx + dz * dz;
		if (dSq < 225.0f) { // 繝・さ繧､蜿榊ｿ懃ｯ・峇
			if (dSq < minDecoyDistSq) {
				minDecoyDistSq = dSq;
				targetPos = decoy->GetPosition();
				chasingDecoy = true;
			}
		}
	}

	if (chasingDecoy) {
		m_currentState = CHASE; // 繝・さ繧､繧定ｿｽ縺・凾縺ｯ辟｡譚｡莉ｶ縺ｧ霑ｽ霍｡
	}
	else {
		// --- 繧ｹ繝・・繝磯・遘ｻ ---
		switch (m_currentState) {
		case PATROL:
			if (canSee || canHear) {
				m_currentState = CHASE;
			}
			else {
				// 逶ｮ逧・慍・・_ambushTarget繧呈ｵ∫畑・峨′縺ｪ縺・√∪縺溘・蛻ｰ逹縺励◆繧画ｬ｡縺ｮ逶ｮ逧・慍繧定ｨｭ螳・
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
				// 隕句､ｱ縺｣縺・-> 謗｢邏｢縺ｸ
				m_currentState = SEARCH;
				m_searchTimer = SEARCH_DURATION;
				targetPos = m_lastKnownPlayerPos; // 譛蠕後↓隕句､ｱ縺｣縺溷ｴ謇縺ｸ陦後￥
			}
			break;

		case SEARCH:
			if (canSee || canHear) {
				m_currentState = CHASE;
			}
			else {
				m_searchTimer -= deltaTime;
				if (m_searchTimer <= 0.0f) {
					// 謗｢邏｢邨ゆｺ・-> 蠕・■莨上○ 縺ｾ縺溘・ 蟾｡蝗・
					// 縺薙％縺ｧ縺ｯ繝ｩ繝ｳ繝繝縺ｫ驛ｨ螻九ｒ驕ｸ繧薙〒蠕・■莨上○縺吶ｋ
					m_currentState = AMBUSH;
					m_ambushTimer = AMBUSH_DURATION;
					
					const auto& rooms = stage->GetRooms();
					if (!rooms.empty()) {
						int idx = static_cast<int>(rand() % rooms.size());
						m_ambushTarget = rooms[idx].center;
						m_ambushTarget.x = m_ambushTarget.x * pathWidth;
						m_ambushTarget.z = m_ambushTarget.z * pathWidth;
					} else {
						m_currentState = PATROL; // 驛ｨ螻九′縺ｪ縺代ｌ縺ｰPATROL
					}
				}
				targetPos = m_lastKnownPlayerPos; // 謗｢邏｢荳ｭ縺ｯ譛蠕後・菴咲ｽｮ縺ｸ蜷代°縺・ら捩縺・◆繧峨え繝ｭ繧ｦ繝ｭ縺輔○縺溘＞縺御ｻ雁屓縺ｯ菴咲ｽｮ邯ｭ謖・
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
	// 繧ｿ繝ｼ繧ｲ繝・ヨ縺悟､峨ｏ繧峨↑縺・剞繧翫√ヱ繧ｹ蜀崎ｨ育ｮ励・繧ｯ繝ｼ繝ｫ繝繧ｦ繝ｳ縺斐→縺ｫ陦後≧
	m_pathCooldown -= deltaTime;
	if (true) { // 蟶ｸ縺ｫ遘ｻ蜍募愛螳壹ｒ陦後≧ (PATROL, SEARCH, AMBUSH, CHASE 蜈ｨ縺ｦ縺ｧ遘ｻ蜍募庄閭ｽ諤ｧ縺後≠繧九◆繧・
		if (m_pathCooldown <= 0.0f) {
			m_pathCooldown = PATH_COOLDOWN_TIME;

			int startX = static_cast<int>(m_position.x / pathWidth);
			int startY = static_cast<int>(m_position.z / pathWidth);
			int goalX = static_cast<int>(targetPos.x / pathWidth);
			int goalY = static_cast<int>(targetPos.z / pathWidth);

			// 繝槭ャ繝礼ｯ・峇螟悶メ繧ｧ繝・け
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
				if (m_currentState == AMBUSH) speed *= 1.5f; // 蜈亥屓繧頑凾縺ｯ騾溘￥遘ｻ蜍・
				if (m_currentState == CHASE && canSee) speed *= 1.2f; // 隕冶ｪ堺ｸｭ縺ｯ蜉騾・

				DirectX::XMVECTOR moveVec = DirectX::XMVector3Normalize(vecToTarget);
				
				// --- 蛻・屬・・eparation・画嫌蜍・---
				DirectX::XMVECTOR separationVec = DirectX::XMVectorZero();
				int neighborCount = 0;
				for (const auto& otherEnemy : enemies) {
					if (otherEnemy.get() == this) continue;

					DirectX::XMFLOAT3 neighborPos = otherEnemy->GetPosition();
					DirectX::XMVECTOR otherPos = DirectX::XMLoadFloat3(&neighborPos);
					DirectX::XMVECTOR toNeighbor = DirectX::XMVectorSubtract(otherPos, DirectX::XMLoadFloat3(&m_position));
					float distToNeighbor = DirectX::XMVectorGetX(DirectX::XMVector3Length(toNeighbor));

					if (distToNeighbor < 2.0f && distToNeighbor > 0.001f) { // 2.0f莉･蜀・・謨ｵ縺九ｉ髮｢繧後ｋ
						DirectX::XMVECTOR flee = DirectX::XMVectorScale(DirectX::XMVector3Normalize(toNeighbor), -1.0f);
						flee = DirectX::XMVectorScale(flee, 1.0f / distToNeighbor); // 霑代＞縺ｻ縺ｩ蠑ｷ縺城屬繧後ｋ
						separationVec = DirectX::XMVectorAdd(separationVec, flee);
						neighborCount++;
					}
				}

				if (neighborCount > 0) {
					separationVec = DirectX::XMVectorScale(separationVec, 1.5f); // 蛻・屬縺ｮ驥阪∩
					moveVec = DirectX::XMVectorAdd(moveVec, separationVec);
					moveVec = DirectX::XMVector3Normalize(moveVec); // 豁｣隕丞喧縺励↑縺翫☆

					// 蜷後§蝣ｴ謇縺ｫ逡吶∪繧顔ｶ壹￠繧九・繧帝亟縺舌◆繧√∵ｷｷ髮第凾縺ｯ遒ｺ邇・〒繧ｿ繝ｼ繧ｲ繝・ヨ螟画峩・亥ｷ｡蝗樊凾縺ｫ髯舌ｋ・・
					if (m_currentState == PATROL && (rand() % 100) < 2) { // 2%縺ｮ遒ｺ邇・〒螟画峩
				 		m_path.clear(); // 蠑ｷ蛻ｶ逧・↓繝代せ繝ｪ繧ｻ繝・ヨ
						m_pathIndex = -1;
					}
				}

				moveVec = DirectX::XMVectorScale(moveVec, speed * deltaTime);
				DirectX::XMStoreFloat3(&m_position, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&m_position), moveVec));

				// --- 繧ｹ繧ｿ繝・け讀懃衍 ---
				float movedDistSq = 
					(m_position.x - m_prevPosition.x) * (m_position.x - m_prevPosition.x) +
					(m_position.z - m_prevPosition.z) * (m_position.z - m_prevPosition.z);
				
				if (movedDistSq < 0.0001f) { // 縺ｻ縺ｨ繧薙←蜍輔＞縺ｦ縺・↑縺・
					m_stuckTimer += deltaTime;
					if (m_stuckTimer > 2.0f) { // 2遘剃ｻ･荳翫せ繧ｿ繝・け
						// 繧ｹ繧ｿ繝・け閼ｱ蜃ｺ・壹Λ繝ｳ繝繝縺ｪ譁ｹ蜷代∈蟆代＠蜍輔°縺吶°縲√ち繝ｼ繧ｲ繝・ヨ繧貞ｼｷ蛻ｶ螟画峩
						m_path.clear();
						m_pathIndex = -1;
						m_ambushTarget = m_position; // 荳譌ｦ繧ｿ繝ｼ繧ｲ繝・ヨ繧定・蛻・↓縺励※繝ｪ繧ｻ繝・ヨ
						m_currentState = PATROL; // 蟾｡蝗槭↓謌ｻ繧・
						m_stuckTimer = 0.0f;
					}
				} else {
					m_stuckTimer = 0.0f; // 蜍輔＞縺溘ｉ繝ｪ繧ｻ繝・ヨ
				}
				m_prevPosition = m_position;
			}
		}
	}

	// --- 繝代・繝・ぅ繧ｯ繝ｫ縺ｮ譖ｴ譁ｰ ---
	m_particleSystem->Update(deltaTime, m_position);
}

// 謠冗判: 繝代・繝・ぅ繧ｯ繝ｫ繧ｷ繧ｹ繝・Β縺ｫ蟋碑ｭｲ
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

	// 繝ｬ繧､繧ｭ繝｣繧ｹ繝・(邁｡譏鍋沿: 繧ｹ繝・ャ繝励し繧､繧ｺ縺斐→縺ｫ螢∝愛螳・
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
				return false; // 螢√↓驕ｮ繧峨ｌ縺・
			}
		}
	}
	return true;
}
