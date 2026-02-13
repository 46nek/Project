#include "GameObjectManager.h"
#include "AssetPaths.h"
#include "Camera.h"
#include "Game.h"
#include <random>
#include <algorithm>
#include <tuple>

namespace {
	constexpr int SPAWN_ROOM_SIZE = 3;
	constexpr int SPAWN_CORNER_OFFSET = 1;
	constexpr float SPAWN_CHECK_DISTANCE = 2.0f;
	const DirectX::XMFLOAT4 COLOR_NORMAL_ORB = { 0.8f, 0.8f, 1.0f, 1.0f };
	const DirectX::XMFLOAT4 COLOR_ZOOM_ORB = { 0.2f, 1.0f, 0.2f, 1.0f };
	const DirectX::XMFLOAT4 COLOR_RADAR_ORB = { 1.0f, 0.2f, 0.2f, 1.0f };
	const DirectX::XMFLOAT4 COLOR_GOAL_ORB = { 1.0f, 0.8f, 0.0f, 1.0f };
	constexpr float ORB_LIGHT_RANGE = 5.0f;
	constexpr float ORB_LIGHT_INTENSITY = 1.0f;
	constexpr float SPECIAL_ORB_INTENSITY = 1.5f;
	constexpr float GOAL_LIGHT_RANGE = 10.0f;
	constexpr float GOAL_LIGHT_INTENSITY = 2.0f;
	constexpr float RADAR_DURATION = 20.0f;
}

GameObjectManager::GameObjectManager()
	: m_remainingOrbs(0), m_totalOrbs(0), m_goalSpawned(false),
	m_escapeMode(false), m_enemyRadarTimer(0.0f),
	m_requestZoomOut(false) {
}

GameObjectManager::~GameObjectManager() {
	Shutdown();
}

void GameObjectManager::Shutdown() {
	for (auto& orb : m_orbs) { if (orb) { orb->Shutdown(); } }
	m_orbs.clear();
	for (auto& enemy : m_enemies) { if (enemy) { enemy->Shutdown(); } }
	m_enemies.clear();
	m_specialOrbs.clear();
	if (m_goalOrb) { m_goalOrb->Shutdown(); }
}

bool GameObjectManager::Initialize(GraphicsDevice* graphicsDevice, Stage* stage, LightManager* lightManager) {
	m_graphicsDevice = graphicsDevice;

	if (!InitializeEnemies(graphicsDevice, stage)) { return false; }
	if (!InitializeOrbs(graphicsDevice, stage, lightManager)) { return false; }
	if (!InitializeSpecialOrbs(graphicsDevice, stage, lightManager)) { return false; }
	return true;
}

bool GameObjectManager::InitializeEnemies(GraphicsDevice* graphicsDevice, Stage* stage) {
	float pathWidth = stage->GetPathWidth();
	std::vector<std::pair<int, int>> spawnRooms = {
		{1, 1},
		{Stage::MAZE_WIDTH - 4, 1},
		{1, Stage::MAZE_HEIGHT - 4},
		{Stage::MAZE_WIDTH - 4, Stage::MAZE_HEIGHT - 4}
	};

	std::random_device rd;
	std::mt19937 gen(rd());
	std::shuffle(spawnRooms.begin(), spawnRooms.end(), gen);

	for (int i = 0; i < NUM_ENEMIES; ++i) {
		std::pair<int, int> room = spawnRooms[i];
		float enemyStartX = (static_cast<float>(room.first) + 1.5f) * pathWidth;
		float enemyStartZ = (static_cast<float>(room.second) + 1.5f) * pathWidth;

		auto enemy = std::make_unique<Enemy>();
		if (!enemy->Initialize(graphicsDevice->GetDevice(), { enemyStartX, 1.0f, enemyStartZ }, stage->GetMazeData())) {
			return false;
		}
		m_enemies.push_back(std::move(enemy));
	}
	return true;
}

bool GameObjectManager::InitializeOrbs(GraphicsDevice* graphicsDevice, Stage* stage, LightManager* lightManager) {
	const auto& mazeData = stage->GetMazeData();
	float pathWidth = stage->GetPathWidth();
	std::vector<std::pair<int, int>> possibleSpawns;

	std::random_device rd;
	std::mt19937 gen(rd());

	using Rect = std::tuple<int, int, int, int>;
	const std::vector<Rect> rooms = {
		std::make_tuple(SPAWN_CORNER_OFFSET, SPAWN_CORNER_OFFSET, SPAWN_ROOM_SIZE, SPAWN_ROOM_SIZE),
		std::make_tuple(Stage::MAZE_WIDTH - SPAWN_CORNER_OFFSET - SPAWN_ROOM_SIZE, SPAWN_CORNER_OFFSET, SPAWN_ROOM_SIZE, SPAWN_ROOM_SIZE),
		std::make_tuple(SPAWN_CORNER_OFFSET, Stage::MAZE_HEIGHT - SPAWN_CORNER_OFFSET - SPAWN_ROOM_SIZE, SPAWN_ROOM_SIZE, SPAWN_ROOM_SIZE),
		std::make_tuple(Stage::MAZE_WIDTH - SPAWN_CORNER_OFFSET - SPAWN_ROOM_SIZE, Stage::MAZE_HEIGHT - SPAWN_CORNER_OFFSET - SPAWN_ROOM_SIZE, SPAWN_ROOM_SIZE, SPAWN_ROOM_SIZE),
		std::make_tuple((Stage::MAZE_WIDTH - SPAWN_ROOM_SIZE) / 2, (Stage::MAZE_HEIGHT - SPAWN_ROOM_SIZE) / 2, SPAWN_ROOM_SIZE, SPAWN_ROOM_SIZE)
	};

	for (size_t y = 1; y < mazeData.size() - 1; ++y) {
		for (size_t x = 1; x < mazeData[0].size() - 1; ++x) {
			if (mazeData[y][x] == MazeGenerator::Path) {
				bool isInRoom = false;
				for (const auto& r : rooms) {
					int sx = std::get<0>(r), sy = std::get<1>(r), w = std::get<2>(r), h = std::get<3>(r);
					if (x >= sx && x < sx + w && y >= sy && y < sy + h) {
						isInRoom = true;
						break;
					}
				}
				if (isInRoom) { continue; }

				int pathNeighbors = 0;
				if (mazeData[y - 1][x] == MazeGenerator::Path) { pathNeighbors++; }
				if (mazeData[y + 1][x] == MazeGenerator::Path) { pathNeighbors++; }
				if (mazeData[y][x - 1] == MazeGenerator::Path) { pathNeighbors++; }
				if (mazeData[y][x + 1] == MazeGenerator::Path) { pathNeighbors++; }

				if (pathNeighbors <= 2) {
					possibleSpawns.push_back({ static_cast<int>(x), static_cast<int>(y) });
				}
			}
		}
	}

	std::shuffle(possibleSpawns.begin(), possibleSpawns.end(), gen);
	std::vector<std::pair<int, int>> spawnedOrbPositions;

	for (const auto& spawnPos : possibleSpawns) {
		if (m_orbs.size() >= NUM_ORBS) { break; }

		bool canSpawn = true;
		for (const auto& spawnedPos : spawnedOrbPositions) {
			if (std::abs(spawnPos.first - spawnedPos.first) + std::abs(spawnPos.second - spawnedPos.second) <= SPAWN_CHECK_DISTANCE) {
				canSpawn = false;
				break;
			}
		}

		if (canSpawn) {
			float orbX = (static_cast<float>(spawnPos.first) + 0.5f) * pathWidth;
			float orbZ = (static_cast<float>(spawnPos.second) + 0.5f) * pathWidth;
			DirectX::XMFLOAT3 orbPos = { orbX, 2.0f, orbZ };

			int lightIndex = lightManager->AddPointLight(orbPos, COLOR_NORMAL_ORB, ORB_LIGHT_RANGE, ORB_LIGHT_INTENSITY);

			if (lightIndex != -1) {
				auto orb = std::make_unique<Orb>();
				if (orb->Initialize(graphicsDevice->GetDevice(), orbPos, lightIndex)) {
					m_orbs.push_back(std::move(orb));
					spawnedOrbPositions.push_back(spawnPos);
				}
			}
		}
	}
	m_totalOrbs = static_cast<int>(m_orbs.size());
	m_remainingOrbs = m_totalOrbs;
	return true;
}

bool GameObjectManager::InitializeSpecialOrbs(GraphicsDevice* graphicsDevice, Stage* stage, LightManager* lightManager) {
	float pathWidth = stage->GetPathWidth();
	std::vector<std::pair<int, int>> cornerRooms = {
		{1, 1},
		{Stage::MAZE_WIDTH - 4, 1},
		{1, Stage::MAZE_HEIGHT - 4},
		{Stage::MAZE_WIDTH - 4, Stage::MAZE_HEIGHT - 4}
	};

	std::random_device rd;
	std::mt19937 gen(rd());
	std::shuffle(cornerRooms.begin(), cornerRooms.end(), gen);

	std::vector<OrbType> specialOrbTypes = { OrbType::MinimapZoomOut, OrbType::EnemyRadar, OrbType::EnemyRadar };

	for (size_t i = 0; i < specialOrbTypes.size(); ++i) {
		std::pair<int, int> room = cornerRooms[i];
		float orbX = (static_cast<float>(room.first) + 1.5f) * pathWidth;
		float orbZ = (static_cast<float>(room.second) + 1.5f) * pathWidth;
		DirectX::XMFLOAT3 orbPos = { orbX, 2.0f, orbZ };

		DirectX::XMFLOAT4 orbColor;
		OrbType type = specialOrbTypes[i];
		switch (type) {
		case OrbType::MinimapZoomOut: orbColor = COLOR_ZOOM_ORB; break;
		case OrbType::EnemyRadar:     orbColor = COLOR_RADAR_ORB; break;
		default:                      orbColor = COLOR_NORMAL_ORB; break;
		}

		int lightIndex = lightManager->AddPointLight(orbPos, orbColor, ORB_LIGHT_RANGE, SPECIAL_ORB_INTENSITY);

		if (lightIndex != -1) {
			auto orb = std::make_unique<Orb>();
			if (orb->Initialize(graphicsDevice->GetDevice(), orbPos, lightIndex, type)) {
				m_specialOrbs.push_back(std::move(orb));
			}
		}
	}
	return true;
}

void GameObjectManager::SpawnGoal(Stage* stage, LightManager* lightManager) {
	// ステージの開始位置（ゴール位置）を取得
	std::pair<int, int> startPos = stage->GetStartPosition();
	float pathWidth = stage->GetPathWidth();

	// 座標計算
	float goalX = (static_cast<float>(startPos.first) + 0.5f) * pathWidth;
	float goalZ = (static_cast<float>(startPos.second) + 0.5f) * pathWidth;

	// ゴールの光を追加（COLOR_GOAL_ORBなどは無名名前空間にある前提）
	int lightIndex = lightManager->AddPointLight({ goalX, 2.0f, goalZ }, COLOR_GOAL_ORB, GOAL_LIGHT_RANGE, GOAL_LIGHT_INTENSITY);

	// ゴールオーブ生成
	m_goalOrb = std::make_unique<Orb>();
	// 保存しておいた m_graphicsDevice を使用
	m_goalOrb->Initialize(m_graphicsDevice->GetDevice(), { goalX, 2.0f, goalZ }, lightIndex, OrbType::Goal);

	m_goalSpawned = true;
}

void GameObjectManager::Update(float deltaTime, Player* player, Stage* stage, LightManager* lightManager, DirectX::SoundEffect* collectSound) {
	//デコイの生成
	if (player->IsDecoyRequested()) {
		auto it_consume = std::find_if(m_orbs.begin(), m_orbs.end(), [](const std::unique_ptr<Orb>& o) {
			return o->IsCollected();
			});

		if (it_consume != m_orbs.end()) {
			auto decoy = std::make_unique<Decoy>((*it_consume)->GetPosition());
			if (decoy->Initialize(m_graphicsDevice->GetDevice())) {
				m_decoys.push_back(std::move(decoy));

				m_remainingOrbs--;

				(*it_consume)->Shutdown();
				m_orbs.erase(it_consume);
			}
		}
		player->ResetDecoyRequest();
	}

	//デコイの更新と有効なデコイのリスト作成
	std::vector<Decoy*> activeDecoyPtrs;
	for (auto it = m_decoys.begin(); it != m_decoys.end(); ) {
		(*it)->Update(deltaTime);
		if (!(*it)->IsActive()) {
			it = m_decoys.erase(it);
		}
		else {
			activeDecoyPtrs.push_back(it->get());
			++it;
		}
	}

	//敵の更新（第5引数にデコイリストを渡す）
	for (auto& enemy : m_enemies) {
		enemy->Update(deltaTime, player, stage, activeDecoyPtrs, m_enemies);
	}

	// 特殊オーブの更新
	for (auto it = m_specialOrbs.begin(); it != m_specialOrbs.end(); ) {
		(*it)->Update(deltaTime, player, lightManager, collectSound);
		if ((*it)->IsCollected()) {
			switch ((*it)->GetType()) {
			case OrbType::MinimapZoomOut:
				m_requestZoomOut = true;
				break;

			case OrbType::EnemyRadar:
				m_enemyRadarTimer = RADAR_DURATION;
				break;
			}
			// 削除せず、Collected状態にするだけに留めるのが一般的ですが、元のロジックに従い削除等の処理を入れます
			it = m_specialOrbs.erase(it);
		}
		else {
			++it;
		}
	}

	// レーダータイマー
	if (m_enemyRadarTimer > 0.0f) {
		m_enemyRadarTimer -= deltaTime;
	}

	DirectX::XMFLOAT3 currentTargetPos = player->GetPosition();

	int heldCount = 0;
	auto it = m_orbs.begin();
	while (it != m_orbs.end()) {
		auto& orb = *it;

		if (!orb->IsCollected()) {
			// 拾っていない場合は当たり判定
			if (orb->Update(deltaTime, player, lightManager, collectSound)) {
				// 拾った瞬間。ここではまだ m_remainingOrbs は減らさない
			}
			++it;
		}
		else {
			orb->FollowPlayer(deltaTime, currentTargetPos, heldCount);

			currentTargetPos = orb->GetPosition(); heldCount++;

			// 中央（納品場所）への距離判定
			auto startGrid = stage->GetStartPosition();
			float centerX = (static_cast<float>(startGrid.first) + 0.5f) * stage->GetPathWidth();
			float centerZ = (static_cast<float>(startGrid.second) + 0.5f) * stage->GetPathWidth();

			float dx = orb->GetPosition().x - centerX;
			float dz = orb->GetPosition().z - centerZ;
			float distSq = dx * dx + dz * dz;

			if (distSq < 1.0f) { // 納品成功
				m_remainingOrbs--; // ここで初めてカウントを減らす
				orb->Shutdown();
				it = m_orbs.erase(it); // リストから消して物理的に消滅させる
			}
			else {
				++it;
			}
		}
	}
	player->SetHeldOrbCount(heldCount);

	// 4. ゴール出現判定
	if (m_remainingOrbs <= 0 && !m_goalSpawned) {
		SpawnGoal(stage, lightManager);
	}

	// 5. ゴール判定（既存のまま）
	if (m_goalSpawned && m_goalOrb && !m_goalOrb->IsCollected()) {
		if (m_goalOrb->Update(deltaTime, player, lightManager, collectSound)) {
			stage->OpenExit();
			m_escapeMode = true;
		}
	}
}

void GameObjectManager::CollectRenderModels(std::vector<Model*>& models) {
	for (const auto& orb : m_orbs) { if (Model* m = orb->GetModel()) { models.push_back(m); } }
	for (const auto& orb : m_specialOrbs) { if (Model* m = orb->GetModel()) { models.push_back(m); } }
	if (m_goalOrb) { if (Model* m = m_goalOrb->GetModel()) { models.push_back(m); } }

	for (const auto& decoy : m_decoys) {
		if (decoy->IsActive()) {
			if (Model* m = decoy->GetModel()) {
				models.push_back(m);
			}
		}
	}
}

bool GameObjectManager::CheckAndResetZoomRequest() {
	if (m_requestZoomOut) {
		m_requestZoomOut = false;
		return true;
	}
	return false;
}

void GameObjectManager::RenderEnemies(GraphicsDevice* graphicsDevice, Camera* camera, LightManager* lightManager) {
	if (!graphicsDevice || !camera || !lightManager) return;

	// プロジェクション行列を作成
	DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
		camera->GetFOV(),
		(float)Game::SCREEN_WIDTH / Game::SCREEN_HEIGHT,
		0.1f,
		1000.0f
	);

	// 全ての敵のRenderを呼び出す
	for (const auto& enemy : m_enemies) {
		if (enemy) {
			enemy->Render(
				graphicsDevice,
				camera->GetViewMatrix(),
				projectionMatrix,
				lightManager->GetLightViewMatrix(),
				lightManager->GetLightProjectionMatrix()
			);
		}
	}
}