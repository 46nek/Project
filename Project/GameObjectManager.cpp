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
	// 繧ｹ繝・・繧ｸ縺ｮ髢句ｧ倶ｽ咲ｽｮ・医ざ繝ｼ繝ｫ菴咲ｽｮ・峨ｒ蜿門ｾ・
	std::pair<int, int> startPos = stage->GetStartPosition();
	float pathWidth = stage->GetPathWidth();

	// 蠎ｧ讓呵ｨ育ｮ・
	float goalX = (static_cast<float>(startPos.first) + 0.5f) * pathWidth;
	float goalZ = (static_cast<float>(startPos.second) + 0.5f) * pathWidth;

	// 繧ｴ繝ｼ繝ｫ縺ｮ蜈峨ｒ霑ｽ蜉・・OLOR_GOAL_ORB縺ｪ縺ｩ縺ｯ辟｡蜷榊錐蜑咲ｩｺ髢薙↓縺ゅｋ蜑肴署・・
	int lightIndex = lightManager->AddPointLight({ goalX, 2.0f, goalZ }, COLOR_GOAL_ORB, GOAL_LIGHT_RANGE, GOAL_LIGHT_INTENSITY);

	// 繧ｴ繝ｼ繝ｫ繧ｪ繝ｼ繝也函謌・
	m_goalOrb = std::make_unique<Orb>();
	// 菫晏ｭ倥＠縺ｦ縺翫＞縺・m_graphicsDevice 繧剃ｽｿ逕ｨ
	m_goalOrb->Initialize(m_graphicsDevice->GetDevice(), { goalX, 2.0f, goalZ }, lightIndex, OrbType::Goal);

	m_goalSpawned = true;
}

void GameObjectManager::Update(float deltaTime, Player* player, Stage* stage, LightManager* lightManager, DirectX::SoundEffect* collectSound) {
	// 繝・さ繧､縺ｮ逕滓・
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

	// 繝・さ繧､縺ｮ譖ｴ譁ｰ縺ｨ譛牙柑縺ｪ繝・さ繧､縺ｮ繝ｪ繧ｹ繝井ｽ懈・
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

	// 謨ｵ縺ｮ譖ｴ譁ｰ・育ｬｬ5蠑墓焚縺ｫ繝・さ繧､繝ｪ繧ｹ繝医ｒ貂｡縺呻ｼ・
	for (auto& enemy : m_enemies) {
		enemy->Update(deltaTime, player, stage, activeDecoyPtrs, m_enemies);
	}

	// 迚ｹ谿翫が繝ｼ繝悶・譖ｴ譁ｰ
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
			// 蜑企勁縺帙★縲，ollected迥ｶ諷九↓縺吶ｋ縺縺代↓縺ｨ縺ｩ繧√ｋ縺ｮ縺御ｸ闊ｬ逧・〒縺吶′縲∽ｻ悶・繝ｭ繧ｸ繝・け縺ｫ蠕薙＞蜑企勁遲峨・蜃ｦ逅・ｒ蜈･繧後∪縺・
			it = m_specialOrbs.erase(it);
		}
		else {
			++it;
		}
	}

	// 繝ｬ繝ｼ繝繝ｼ繧ｿ繧､繝槭・
	if (m_enemyRadarTimer > 0.0f) {
		m_enemyRadarTimer -= deltaTime;
	}

	DirectX::XMFLOAT3 currentTargetPos = player->GetPosition();

	int heldCount = 0;
	auto it = m_orbs.begin();
	while (it != m_orbs.end()) {
		auto& orb = *it;

		if (!orb->IsCollected()) {
			// 諡ｾ縺｣縺ｦ縺・↑縺・ｴ蜷医・蠖薙◆繧雁愛螳・
			if (orb->Update(deltaTime, player, lightManager, collectSound)) {
				// 諡ｾ縺｣縺溽椪髢薙ゅ％縺薙〒縺ｯ縺ｾ縺 m_remainingOrbs 縺ｯ貂帙ｉ縺輔↑縺・
			}
			++it;
		}
		else {
			orb->FollowPlayer(deltaTime, currentTargetPos, heldCount);

			currentTargetPos = orb->GetPosition(); heldCount++;

			// 荳ｭ螟ｮ・育ｴ榊刀蝣ｴ謇・峨∈縺ｮ霍晞屬蛻､螳・
			auto startGrid = stage->GetStartPosition();
			float centerX = (static_cast<float>(startGrid.first) + 0.5f) * stage->GetPathWidth();
			float centerZ = (static_cast<float>(startGrid.second) + 0.5f) * stage->GetPathWidth();

			float dx = orb->GetPosition().x - centerX;
			float dz = orb->GetPosition().z - centerZ;
			float distSq = dx * dx + dz * dz;

			if (distSq < 1.0f) { // 邏榊刀謌仙粥
				m_remainingOrbs--; // 縺薙％縺ｧ蛻昴ａ縺ｦ繧ｫ繧ｦ繝ｳ繝医ｒ貂帙ｉ縺・
				orb->Shutdown();
				it = m_orbs.erase(it); // 繝ｪ繧ｹ繝医°繧画ｶ医＠縺ｦ迚ｩ逅・噪縺ｫ豸域ｻ・＆縺帙ｋ
			}
			else {
				++it;
			}
		}
	}
	player->SetHeldOrbCount(heldCount);

	// 4. 繧ｴ繝ｼ繝ｫ蜃ｺ迴ｾ蛻､螳・
	if (m_remainingOrbs <= 0 && !m_goalSpawned) {
		SpawnGoal(stage, lightManager);
	}

	// 5. 繧ｴ繝ｼ繝ｫ蛻､螳夲ｼ域里蟄倥・縺ｾ縺ｾ・・
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

	// 繝励Ο繧ｸ繧ｧ繧ｯ繧ｷ繝ｧ繝ｳ陦悟・繧剃ｽ懈・
	DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
		camera->GetFOV(),
		(float)Game::SCREEN_WIDTH / Game::SCREEN_HEIGHT,
		0.1f,
		1000.0f
	);

	// 蜈ｨ縺ｦ縺ｮ謨ｵ縺ｮRender繧貞他縺ｳ蜃ｺ縺・
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
