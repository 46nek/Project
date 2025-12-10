#include "GameScene.h"
#include "AssetLoader.h"
#include "Game.h"
#include "Easing.h"
#include "AssetPaths.h"
#include <random>
#include <tuple>
#include <algorithm>

std::unique_ptr<GameScene> GameScene::s_transferInstance = nullptr;

// グローバル変数のGameインスタンスを参照
extern Game* g_game;

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
	constexpr float VIGNETTE_MIN_INTENSITY = 1.1f;
	constexpr float VIGNETTE_MAX_INTENSITY = 2.0f;
	constexpr float VIGNETTE_WARNING_THRESHOLD = 0.3f;
	constexpr float MINIMAP_ZOOM_OUT_LEVEL = 2.0f;
	constexpr float RADAR_DURATION = 20.0f;
	struct BobbingParams {
		float speed, amount, swaySpeed, swayAmount, rollSpeed;
	};
	constexpr BobbingParams BOB_WALK = { 14.0f, 0.03f, 7.0f, 0.05f, 7.0f };
	constexpr BobbingParams BOB_RUN = { 18.0f, 0.05f, 10.0f, 0.08f, 9.0f };
}

GameScene::GameScene()
	: m_vignetteIntensity(0.0f),
	m_remainingOrbs(0),
	m_totalOrbs(0),
	m_enemyRadarTimer(0.0f),
	m_goalSpawned(false),
	m_escapeMode(false),
	m_isOpening(false),
	m_openingTimer(0.0f),
	m_openingDuration(2.5f),
	m_titleTimer(0.0f),
	m_uiFadeTimer(0.0f)
{
}

GameScene::~GameScene() {}

bool GameScene::Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine)
{
	// ゲーム開始時：カーソルを消して中央固定
	input->SetCursorLock(true);

	m_uiFadeTimer = 0.0f;

	if (s_transferInstance)
	{
		m_graphicsDevice = graphicsDevice;
		m_input = input;
		m_audioEngine = audioEngine;

		m_stage = std::move(s_transferInstance->m_stage);
		m_player = std::move(s_transferInstance->m_player);
		m_camera = std::move(s_transferInstance->m_camera);
		m_lightManager = std::move(s_transferInstance->m_lightManager);
		m_renderer = std::move(s_transferInstance->m_renderer);
		m_ui = std::move(s_transferInstance->m_ui);

		m_enemies = std::move(s_transferInstance->m_enemies);
		m_orbs = std::move(s_transferInstance->m_orbs);
		m_specialOrbs = std::move(s_transferInstance->m_specialOrbs);
		m_goalOrb = std::move(s_transferInstance->m_goalOrb);

		m_collectSound = std::move(s_transferInstance->m_collectSound);
		m_walkSoundEffect = std::move(s_transferInstance->m_walkSoundEffect);
		m_runSoundEffect = std::move(s_transferInstance->m_runSoundEffect);

		m_cachedStageModels = s_transferInstance->m_cachedStageModels;
		m_cachedDynamicModels = s_transferInstance->m_cachedDynamicModels;
		m_remainingOrbs = s_transferInstance->m_remainingOrbs;
		m_totalOrbs = s_transferInstance->m_totalOrbs;
		m_goalSpawned = s_transferInstance->m_goalSpawned;
		m_escapeMode = s_transferInstance->m_escapeMode;
		m_enemyRadarTimer = s_transferInstance->m_enemyRadarTimer;
		m_vignetteIntensity = s_transferInstance->m_vignetteIntensity;

		m_isOpening = s_transferInstance->m_isOpening;
		m_openingTimer = s_transferInstance->m_openingTimer;
		m_openingDuration = s_transferInstance->m_openingDuration;
		m_titleCamPos = s_transferInstance->m_titleCamPos;
		m_titleCamRot = s_transferInstance->m_titleCamRot;
		m_startCamPos = s_transferInstance->m_startCamPos;
		m_startCamRot = s_transferInstance->m_startCamRot;
		m_titleTimer = s_transferInstance->m_titleTimer;

		if (m_titleCamPos.y != 0.0f)
		{
			BeginOpening();
		}

		s_transferInstance.reset();

		return true;
	}

	if (!InitializePhase1(graphicsDevice, input, audioEngine)) return false;
	if (!InitializePhase2()) return false;
	if (!InitializePhase3()) return false;
	if (!InitializePhase4()) return false;
	if (!InitializePhase5()) return false;
	return true;
}

bool GameScene::InitializePhase1(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine)
{
	m_graphicsDevice = graphicsDevice;
	m_input = input;
	m_audioEngine = audioEngine;

	m_stage = std::make_unique<Stage>();
	if (!m_stage->Initialize(graphicsDevice)) return false;

	std::pair<int, int> startPos = m_stage->GetStartPosition();
	float pathWidth = m_stage->GetPathWidth();
	float startX = (static_cast<float>(startPos.first) + 0.5f) * pathWidth;
	float startZ = (static_cast<float>(startPos.second) + 0.5f) * pathWidth;

	m_player = std::make_unique<Player>();

	m_camera = std::make_unique<Camera>(startX, PLAYER_HEIGHT, startZ);

	m_startCamPos = { startX, PLAYER_HEIGHT, startZ };
	m_startCamRot = { 0.0f, 180.0f, 0.0f };
	m_camera->SetRotation(0.0f, 180.0f, 0.0f);

	m_lightManager = std::make_unique<LightManager>();
	m_lightManager->Initialize(m_stage->GetMazeData(), m_stage->GetPathWidth(), Stage::WALL_HEIGHT);
	m_renderer = std::make_unique<Renderer>(m_graphicsDevice);

	m_player->Initialize({ startX, PLAYER_HEIGHT, startZ });
	m_player->SetRotation({ 0.0f, 180.0f, 0.0f });

	m_cachedStageModels.clear();
	m_cachedStageModels.reserve(m_stage->GetModels().size());
	for (const auto& model : m_stage->GetModels()) {
		m_cachedStageModels.push_back(model.get());
	}
	return true;
}

bool GameScene::InitializePhase2()
{
	m_ui = std::make_unique<UI>();
	if (!m_ui->Initialize(m_graphicsDevice, m_stage->GetMazeData(), m_stage->GetPathWidth())) return false;
	return true;
}

bool GameScene::InitializePhase3()
{
	return InitializeEnemies();
}

bool GameScene::InitializePhase4()
{
	if (!InitializeOrbs()) return false;
	if (!InitializeSpecialOrbs()) return false;
	return true;
}

bool GameScene::InitializePhase5()
{
	try
	{
		m_collectSound = std::make_unique<DirectX::SoundEffect>(m_audioEngine, AssetPaths::SOUND_ORB_GET);
		m_walkSoundEffect = std::make_unique<DirectX::SoundEffect>(m_audioEngine, AssetPaths::SOUND_WALK);
		m_runSoundEffect = std::make_unique<DirectX::SoundEffect>(m_audioEngine, AssetPaths::SOUND_WALK);

		if (m_player)
		{
			m_player->SetFootstepSounds(m_walkSoundEffect.get(), m_runSoundEffect.get());
		}
	}
	catch (const std::exception& e)
	{
		MessageBoxA(nullptr, e.what(), "Sound Error", MB_OK);
		return false;
	}
	return true;
}

void GameScene::SetCameraForTitle()
{
	if (!m_stage || !m_camera) return;

	std::pair<int, int> startPos = m_stage->GetStartPosition();
	float pathWidth = m_stage->GetPathWidth();
	float centerX = (static_cast<float>(startPos.first) + 0.5f) * pathWidth;

	float titleX = centerX;
	float titleY = 5.0f;
	float titleZ = 4.5f * pathWidth;

	m_camera->SetPosition(titleX, titleY, titleZ);
	m_camera->SetRotation(0.0f, 180.0f, 0.0f);

	m_camera->Update();
	m_camera->Update();

	m_titleCamPos = { titleX, titleY, titleZ };
	m_titleCamRot = { 0.0f, 180.0f, 0.0f };

	UpdateTitleLoop(0.0f);
}

void GameScene::UpdateTitleLoop(float deltaTime)
{
	m_titleTimer += deltaTime;

	if (m_lightManager && m_camera)
	{
		DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PI / 4.0f, (float)Game::SCREEN_WIDTH / Game::SCREEN_HEIGHT, 0.1f, 1000.0f);
		m_lightManager->Update(deltaTime, m_camera->GetViewMatrix(), projectionMatrix, m_camera->GetPosition(), m_camera->GetRotation());
	}
}

void GameScene::BeginOpening()
{
	m_isOpening = true;
	m_openingTimer = 0.0f;
	m_uiFadeTimer = 0.0f;

	if (m_camera)
	{
		m_camera->SetPosition(m_titleCamPos.x, m_titleCamPos.y, m_titleCamPos.z);
		m_camera->SetRotation(m_titleCamRot.x, m_titleCamRot.y, m_titleCamRot.z);
	}
}

void GameScene::UpdateOpening(float deltaTime)
{
	m_openingTimer += deltaTime;
	float t = m_openingTimer / m_openingDuration;

	if (t >= 1.0f)
	{
		t = 1.0f;
		m_isOpening = false;
		m_uiFadeTimer = 0.0f;
	}

	float easeT = Easing::GetValue(EasingType::EaseInOutExpo, t);

	float x = m_titleCamPos.x + (m_startCamPos.x - m_titleCamPos.x) * easeT;
	float y = m_titleCamPos.y + (m_startCamPos.y - m_titleCamPos.y) * easeT;
	float z = m_titleCamPos.z + (m_startCamPos.z - m_titleCamPos.z) * easeT;

	float currentYaw = m_titleCamRot.y;
	float targetYaw = m_startCamRot.y;

	if (targetYaw - currentYaw > 180.0f) currentYaw += 360.0f;
	if (targetYaw - currentYaw < -180.0f) currentYaw -= 360.0f;

	float rx = m_titleCamRot.x + (m_startCamRot.x - m_titleCamRot.x) * easeT;
	float ry = currentYaw + (targetYaw - currentYaw) * easeT;
	float rz = m_titleCamRot.z + (m_startCamRot.z - m_titleCamRot.z) * easeT;

	m_camera->SetPosition(x, y, z);
	m_camera->SetRotation(rx, ry, rz);
	m_camera->Update();

	m_vignetteIntensity = t * VIGNETTE_MIN_INTENSITY;

	if (m_lightManager)
	{
		DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PI / 4.0f, (float)Game::SCREEN_WIDTH / Game::SCREEN_HEIGHT, 0.1f, 1000.0f);
		m_lightManager->Update(100.0f, m_camera->GetViewMatrix(), projectionMatrix, m_camera->GetPosition(), m_camera->GetRotation());
	}
}

bool GameScene::InitializeEnemies()
{
	float pathWidth = m_stage->GetPathWidth();
	std::vector<std::pair<int, int>> spawnRooms = {
		{1, 1},
		{Stage::MAZE_WIDTH - 4, 1},
		{1, Stage::MAZE_HEIGHT - 4},
		{Stage::MAZE_WIDTH - 4, Stage::MAZE_HEIGHT - 4}
	};

	std::random_device rd;
	std::mt19937 gen(rd());
	std::shuffle(spawnRooms.begin(), spawnRooms.end(), gen);

	for (int i = 0; i < NUM_ENEMIES; ++i)
	{
		std::pair<int, int> room = spawnRooms[i];
		float enemyStartX = (static_cast<float>(room.first) + 1.5f) * pathWidth;
		float enemyStartZ = (static_cast<float>(room.second) + 1.5f) * pathWidth;

		auto enemy = std::make_unique<Enemy>();
		if (!enemy->Initialize(m_graphicsDevice->GetDevice(), { enemyStartX, 1.0f, enemyStartZ }, m_stage->GetMazeData()))
		{
			return false;
		}
		m_enemies.push_back(std::move(enemy));
	}
	return true;
}

bool GameScene::InitializeOrbs()
{
	const auto& mazeData = m_stage->GetMazeData();
	float pathWidth = m_stage->GetPathWidth();
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

	for (size_t y = 1; y < mazeData.size() - 1; ++y)
	{
		for (size_t x = 1; x < mazeData[0].size() - 1; ++x)
		{
			if (mazeData[y][x] == MazeGenerator::Path)
			{
				bool isInRoom = false;
				for (const auto& r : rooms) {
					int sx = std::get<0>(r), sy = std::get<1>(r), w = std::get<2>(r), h = std::get<3>(r);
					if (x >= sx && x < sx + w && y >= sy && y < sy + h) {
						isInRoom = true;
						break;
					}
				}
				if (isInRoom) continue;

				int pathNeighbors = 0;
				if (mazeData[y - 1][x] == MazeGenerator::Path) pathNeighbors++;
				if (mazeData[y + 1][x] == MazeGenerator::Path) pathNeighbors++;
				if (mazeData[y][x - 1] == MazeGenerator::Path) pathNeighbors++;
				if (mazeData[y][x + 1] == MazeGenerator::Path) pathNeighbors++;

				if (pathNeighbors <= 2)
				{
					possibleSpawns.push_back({ static_cast<int>(x), static_cast<int>(y) });
				}
			}
		}
	}

	std::shuffle(possibleSpawns.begin(), possibleSpawns.end(), gen);
	std::vector<std::pair<int, int>> spawnedOrbPositions;

	for (const auto& spawnPos : possibleSpawns)
	{
		if (m_orbs.size() >= NUM_ORBS) break;

		bool canSpawn = true;
		for (const auto& spawnedPos : spawnedOrbPositions)
		{
			if (std::abs(spawnPos.first - spawnedPos.first) + std::abs(spawnPos.second - spawnedPos.second) <= SPAWN_CHECK_DISTANCE)
			{
				canSpawn = false;
				break;
			}
		}

		if (canSpawn)
		{
			float orbX = (static_cast<float>(spawnPos.first) + 0.5f) * pathWidth;
			float orbZ = (static_cast<float>(spawnPos.second) + 0.5f) * pathWidth;
			DirectX::XMFLOAT3 orbPos = { orbX, 2.0f, orbZ };

			int lightIndex = m_lightManager->AddPointLight(orbPos, COLOR_NORMAL_ORB, ORB_LIGHT_RANGE, ORB_LIGHT_INTENSITY);

			if (lightIndex != -1)
			{
				auto orb = std::make_unique<Orb>();
				if (orb->Initialize(m_graphicsDevice->GetDevice(), orbPos, lightIndex))
				{
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

bool GameScene::InitializeSpecialOrbs()
{
	float pathWidth = m_stage->GetPathWidth();
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

	for (size_t i = 0; i < specialOrbTypes.size(); ++i)
	{
		std::pair<int, int> room = cornerRooms[i];
		float orbX = (static_cast<float>(room.first) + 1.5f) * pathWidth;
		float orbZ = (static_cast<float>(room.second) + 1.5f) * pathWidth;
		DirectX::XMFLOAT3 orbPos = { orbX, 2.0f, orbZ };

		DirectX::XMFLOAT4 orbColor;
		OrbType type = specialOrbTypes[i];
		switch (type)
		{
		case OrbType::MinimapZoomOut: orbColor = COLOR_ZOOM_ORB; break;
		case OrbType::EnemyRadar:     orbColor = COLOR_RADAR_ORB; break;
		default:                      orbColor = COLOR_NORMAL_ORB; break;
		}

		int lightIndex = m_lightManager->AddPointLight(orbPos, orbColor, ORB_LIGHT_RANGE, SPECIAL_ORB_INTENSITY);

		if (lightIndex != -1)
		{
			auto orb = std::make_unique<Orb>();
			if (orb->Initialize(m_graphicsDevice->GetDevice(), orbPos, lightIndex, type))
			{
				m_specialOrbs.push_back(std::move(orb));
			}
		}
	}
	return true;
}

void GameScene::Shutdown()
{
	if (m_player) m_player->SetFootstepSounds(nullptr, nullptr);
	if (m_ui) m_ui->Shutdown();

	for (auto& orb : m_orbs) if (orb) orb->Shutdown();
	m_orbs.clear();

	for (auto& enemy : m_enemies) if (enemy) enemy->Shutdown();
	m_enemies.clear();

	if (m_stage) m_stage->Shutdown();
}

void GameScene::Update(float deltaTime)
{
	if (m_isOpening)
	{
		UpdateOpening(deltaTime);

		return;
	}
	// ESCキーが押されたらカーソルを表示（ロック解除）
	if (m_input->IsKeyPressed(VK_ESCAPE))
	{
		m_input->SetCursorLock(false);
	}
	// 左クリックされたらカーソルを非表示（ロック）
	// Inputクラスでクリックが取れない場合に備え、GetAsyncKeyStateも併用
	else if (m_input->IsKeyPressed(VK_LBUTTON) || (GetAsyncKeyState(VK_LBUTTON) & 0x8000))
	{
		m_input->SetCursorLock(true);
	}
	if (m_uiFadeTimer < UI_FADE_DURATION)
	{
		m_uiFadeTimer += deltaTime;
		if (m_uiFadeTimer > UI_FADE_DURATION) m_uiFadeTimer = UI_FADE_DURATION;
	}

	int mouseX, mouseY;
	m_input->GetMouseDelta(mouseX, mouseY);
	m_player->Turn(mouseX, mouseY, deltaTime);
	m_player->Update(deltaTime, m_input, m_stage->GetMazeData(), m_stage->GetPathWidth());

	UpdateVignette(m_player->GetStaminaPercentage());

	for (auto& enemy : m_enemies) enemy->Update(deltaTime, m_player.get(), m_stage->GetMazeData(), m_stage->GetPathWidth());

	for (auto& orb : m_orbs)
	{
		if (orb && !orb->IsCollected())
		{
			bool justCollected = orb->Update(deltaTime, m_player.get(), m_lightManager.get(), m_collectSound.get());
			if (justCollected)
			{
				m_remainingOrbs--;
			}
		}
	}

	for (auto it = m_specialOrbs.begin(); it != m_specialOrbs.end(); )
	{
		(*it)->Update(deltaTime, m_player.get(), m_lightManager.get(), m_collectSound.get());
		if ((*it)->IsCollected())
		{
			switch ((*it)->GetType())
			{
			case OrbType::MinimapZoomOut:
				if (m_ui) m_ui->SetMinimapZoom(MINIMAP_ZOOM_OUT_LEVEL);
				break;
			case OrbType::EnemyRadar:
				m_enemyRadarTimer = RADAR_DURATION;
				break;
			}
			it = m_specialOrbs.erase(it);
		}
		else
		{
			++it;
		}
	}

	if (m_remainingOrbs <= 0 && !m_goalSpawned)
	{
		std::pair<int, int> startPos = m_stage->GetStartPosition();
		float pathWidth = m_stage->GetPathWidth();
		float goalX = (static_cast<float>(startPos.first) + 0.5f) * pathWidth;
		float goalZ = (static_cast<float>(startPos.second) + 0.5f) * pathWidth;

		int lightIndex = m_lightManager->AddPointLight({ goalX, 2.0f, goalZ }, COLOR_GOAL_ORB, GOAL_LIGHT_RANGE, GOAL_LIGHT_INTENSITY);

		m_goalOrb = std::make_unique<Orb>();
		m_goalOrb->Initialize(m_graphicsDevice->GetDevice(), { goalX, 2.0f, goalZ }, lightIndex, OrbType::Goal);

		m_goalSpawned = true;
	}

	if (m_goalSpawned && m_goalOrb && !m_goalOrb->IsCollected())
	{
		if (m_goalOrb->Update(deltaTime, m_player.get(), m_lightManager.get(), m_collectSound.get()))
		{
			m_stage->OpenExit();
			m_escapeMode = true;
		}
	}

	if (m_escapeMode)
	{
		DirectX::XMFLOAT3 pPos = m_player->GetPosition();
		if (pPos.z < m_stage->GetPathWidth() * 0.8f)
		{
			m_nextScene = SceneState::Result;
		}
	}

	if (m_enemyRadarTimer > 0.0f)
	{
		m_enemyRadarTimer -= deltaTime;
	}

	DirectX::XMFLOAT3 playerPos = m_player->GetPosition();
	DirectX::XMFLOAT3 playerRot = m_player->GetRotation();
	m_camera->SetPosition(playerPos.x, playerPos.y, playerPos.z);
	m_camera->SetRotation(playerRot.x, playerRot.y, playerRot.z);

	const BobbingParams& bp = m_player->IsRunning() ? BOB_RUN : BOB_WALK;
	m_camera->SetBobbingParameters(bp.speed, bp.amount, bp.swaySpeed, bp.swayAmount, bp.rollSpeed);

	m_camera->UpdateBobbing(deltaTime, m_player->IsMoving());
	m_camera->Update();

	DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PI / 4.0f, (float)Game::SCREEN_WIDTH / Game::SCREEN_HEIGHT, 0.1f, 1000.0f);
	m_lightManager->Update(deltaTime, m_camera->GetViewMatrix(), projectionMatrix, m_camera->GetPosition(), m_camera->GetRotation());
	m_ui->Update(deltaTime, m_remainingOrbs, m_totalOrbs, m_player->GetStaminaPercentage(), m_enemyRadarTimer > 0.0f);
}

void GameScene::UpdateVignette(float staminaPercentage)
{
	if (staminaPercentage > VIGNETTE_WARNING_THRESHOLD)
	{
		m_vignetteIntensity = VIGNETTE_MIN_INTENSITY;
	}
	else
	{
		float factorInWarningZone = staminaPercentage / VIGNETTE_WARNING_THRESHOLD;
		float intensityFactor = 1.0f - factorInWarningZone;
		m_vignetteIntensity = VIGNETTE_MIN_INTENSITY + (VIGNETTE_MAX_INTENSITY - VIGNETTE_MIN_INTENSITY) * intensityFactor;
	}
}

void GameScene::RenderStageOnly()
{
	m_cachedDynamicModels.clear();
	size_t dynamicCount = m_enemies.size() + m_orbs.size() + m_specialOrbs.size() + (m_goalOrb ? 1 : 0) + 1;
	m_cachedDynamicModels.reserve(dynamicCount);

	for (const auto& enemy : m_enemies) m_cachedDynamicModels.push_back(enemy->GetModel());
	for (const auto& orb : m_orbs) if (Model* orbModel = orb->GetModel()) m_cachedDynamicModels.push_back(orbModel);
	for (const auto& sorb : m_specialOrbs) if (Model* orbModel = sorb->GetModel()) m_cachedDynamicModels.push_back(orbModel);

	if (m_goalOrb)
	{
		if (Model* gModel = m_goalOrb->GetModel()) m_cachedDynamicModels.push_back(gModel);
	}

	if (Model* gate = m_stage->GetGateModel())
	{
		m_cachedDynamicModels.push_back(gate);
	}

	m_renderer->RenderSceneToTexture(
		m_cachedStageModels,
		m_cachedDynamicModels,
		m_camera.get(),
		m_lightManager.get(),
		m_stage->GetMazeData(),
		m_stage->GetPathWidth()
	);
	m_renderer->RenderFinalPass(m_camera.get(), m_vignetteIntensity);
}

void GameScene::Render()
{
	RenderStageOnly();

	if (m_ui)
	{
		float uiAlpha = 1.0f;
		if (m_isOpening)
		{
			uiAlpha = 0.0f;
		}
		else
		{
			uiAlpha = m_uiFadeTimer / UI_FADE_DURATION;
			if (uiAlpha > 1.0f) uiAlpha = 1.0f;
		}

		m_ui->Render(m_camera.get(), m_enemies, m_orbs, m_specialOrbs, uiAlpha);
	}

	m_graphicsDevice->EndScene();
}