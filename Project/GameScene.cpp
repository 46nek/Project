#include "GameScene.h"
#include "AssetLoader.h"
#include "Game.h"
#include <random>
#include <tuple>
#include <algorithm>

GameScene::GameScene()
	: m_vignetteIntensity(0.0f),
	m_remainingOrbs(0),
	m_totalOrbs(0)
{
}

GameScene::~GameScene() {}
bool GameScene::Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine)
{
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
	m_lightManager = std::make_unique<LightManager>();
	m_lightManager->Initialize(m_stage->GetMazeData(), m_stage->GetPathWidth(), Stage::WALL_HEIGHT);
	m_renderer = std::make_unique<Renderer>(m_graphicsDevice);
	m_player->Initialize({ startX, PLAYER_HEIGHT, startZ });

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
	if (!InitializeEnemies()) return false;
	return true;
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
		m_collectSound = std::make_unique<DirectX::SoundEffect>(m_audioEngine, L"Assets/orb_get.wav");
		m_walkSoundEffect = std::make_unique<DirectX::SoundEffect>(m_audioEngine, L"Assets/walk.wav");
		m_runSoundEffect = std::make_unique<DirectX::SoundEffect>(m_audioEngine, L"Assets/walk.wav");

		// プレイヤーに音をセット
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

	const int roomSize = 3;
	const int cornerOffset = 1;
	using Rect = std::tuple<int, int, int, int>;
	const std::vector<Rect> rooms = {
		std::make_tuple(cornerOffset, cornerOffset, roomSize, roomSize),
		std::make_tuple(Stage::MAZE_WIDTH - cornerOffset - roomSize, cornerOffset, roomSize, roomSize),
		std::make_tuple(cornerOffset, Stage::MAZE_HEIGHT - cornerOffset - roomSize, roomSize, roomSize),
		std::make_tuple(Stage::MAZE_WIDTH - cornerOffset - roomSize, Stage::MAZE_HEIGHT - cornerOffset - roomSize, roomSize, roomSize),
		std::make_tuple((Stage::MAZE_WIDTH - roomSize) / 2, (Stage::MAZE_HEIGHT - roomSize) / 2, roomSize, roomSize)
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
			if (std::abs(spawnPos.first - spawnedPos.first) + std::abs(spawnPos.second - spawnedPos.second) <= 2)
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

			DirectX::XMFLOAT4 orbColor = { 0.8f, 0.8f, 1.0f, 1.0f };
			float orbRange = 5.0f;
			float orbIntensity = 1.0f;
			int lightIndex = m_lightManager->AddPointLight(orbPos, orbColor, orbRange, orbIntensity);

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

	// オーブの種類リスト
	std::vector<OrbType> specialOrbTypes = { OrbType::MinimapZoomOut, OrbType::EnemyRadar, OrbType::EnemyRadar };

	for (size_t i = 0; i < specialOrbTypes.size(); ++i)
	{
		std::pair<int, int> room = cornerRooms[i];
		float orbX = (static_cast<float>(room.first) + 1.5f) * pathWidth; // 部屋の中心あたり
		float orbZ = (static_cast<float>(room.second) + 1.5f) * pathWidth;
		DirectX::XMFLOAT3 orbPos = { orbX, 2.0f, orbZ };

		DirectX::XMFLOAT4 orbColor;
		OrbType type = specialOrbTypes[i];
		switch (type)
		{
		case OrbType::MinimapZoomOut:
			orbColor = { 0.2f, 1.0f, 0.2f, 1.0f }; // 緑
			break;
		case OrbType::EnemyRadar:
			orbColor = { 1.0f, 0.2f, 0.2f, 1.0f }; // 赤
			break;
		default:
			orbColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			break;
		}

		int lightIndex = m_lightManager->AddPointLight(orbPos, orbColor, 5.0f, 1.5f);

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
	if (m_ui) m_ui->Shutdown();
	for (auto& orb : m_orbs) if (orb) orb->Shutdown();
	m_orbs.clear();
	for (auto& enemy : m_enemies) if (enemy) enemy->Shutdown();
	m_enemies.clear();
	if (m_stage) m_stage->Shutdown();
}

void GameScene::Update(float deltaTime)
{
	int mouseX, mouseY;
	m_input->GetMouseDelta(mouseX, mouseY);
	m_player->Turn(mouseX, mouseY, deltaTime);
	m_player->Update(deltaTime, m_input, m_stage->GetMazeData(), m_stage->GetPathWidth());
	
	float staminaPercentage = m_player->GetStaminaPercentage();

	const float minIntensity = 1.1f;
	const float maxIntensity = 2.0f;
	const float warningThreshold = 0.3f;

	if (staminaPercentage > warningThreshold)
	{
		m_vignetteIntensity = minIntensity;
	}
	else
	{
		float factorInWarningZone = staminaPercentage / warningThreshold;
		float intensityFactor = 1.0f - factorInWarningZone;
		m_vignetteIntensity = minIntensity + (maxIntensity - minIntensity) * intensityFactor;
	}
	
	for (auto& enemy : m_enemies) enemy->Update(deltaTime, m_player.get(), m_stage->GetMazeData(), m_stage->GetPathWidth());
	for (auto& orb : m_orbs) 
	{
		// 収集済みでなければ更新処理をかける
		if (orb && !orb->IsCollected()) //
		{
			bool justCollected = orb->Update(deltaTime, m_player.get(), m_lightManager.get(), m_collectSound.get());
			if (justCollected)
			{
				m_remainingOrbs--;
			}
		}
	}
	// 特殊オーブの更新と当たり判定
	for (auto it = m_specialOrbs.begin(); it != m_specialOrbs.end(); )
	{
		(*it)->Update(deltaTime, m_player.get(), m_lightManager.get(), m_collectSound.get());
		if ((*it)->IsCollected())
		{
			// オーブの効果を発動
			switch ((*it)->GetType())
			{
			case OrbType::MinimapZoomOut:
				if (m_ui) m_ui->GetMinimap()->SetZoom(2.0f); // ズームアウト（値は適宜調整）
				break;
			case OrbType::EnemyRadar:
				m_enemyRadarTimer = 20.0f; // 20秒タイマーセット
				break;
			}
			it = m_specialOrbs.erase(it); // 取得したオーブをリストから削除
		}
		else
		{
			++it;
		}
	}

	if (m_remainingOrbs <= 0 && !m_goalSpawned)
	{
		// スタート地点にゴールオーブを出現させる
		std::pair<int, int> startPos = m_stage->GetStartPosition();
		float pathWidth = m_stage->GetPathWidth();
		float goalX = (static_cast<float>(startPos.first) + 0.5f) * pathWidth;
		float goalZ = (static_cast<float>(startPos.second) + 0.5f) * pathWidth;

		// 目立つように強い光を放つ
		int lightIndex = m_lightManager->AddPointLight({ goalX, 2.0f, goalZ }, { 1.0f, 0.8f, 0.0f, 1.0f }, 10.0f, 2.0f);

		m_goalOrb = std::make_unique<Orb>();
		m_goalOrb->Initialize(m_graphicsDevice->GetDevice(), { goalX, 2.0f, goalZ }, lightIndex, OrbType::Goal);

		m_goalSpawned = true;

		// ここで「脱出への道が開かれるぞ！」などの音やメッセージを出すと良いでしょう
	}

	// ▼▼▼ 追加: ゴールオーブの更新と壁開放 ▼▼▼
	if (m_goalSpawned && m_goalOrb && !m_goalOrb->IsCollected())
	{
		if (m_goalOrb->Update(deltaTime, m_player.get(), m_lightManager.get(), m_collectSound.get()))
		{
			// ゴールオーブを取得した！ -> 出口を開ける
			m_stage->OpenExit();
			m_escapeMode = true;
			// ここでBGMをチェイス曲に変えるなどの演出を入れる
		}
	}

	// ▼▼▼ 追加: 脱出判定（クリア判定） ▼▼▼
	if (m_escapeMode)
	{
		// 出口座標（Z=0付近）に到達したかチェック
		// m_stage->OpenExit() で出口の壁判定は消えているので、プレイヤーはそこに入ることができる
		DirectX::XMFLOAT3 pPos = m_player->GetPosition();
		if (pPos.z < m_stage->GetPathWidth() * 0.8f) // 一番上の行（Z=0〜2.5）に入り込んだらクリア
		{
			MessageBoxA(nullptr, "ESCAPED! YOU WIN!", "CONGRATULATIONS", MB_OK);
			// ゲームクリア処理（タイトルに戻るなど）
		}
	}

	// 敵レーダーのタイマー更新
	if (m_enemyRadarTimer > 0.0f)
	{
		m_enemyRadarTimer -= deltaTime;
	}

	DirectX::XMFLOAT3 playerPos = m_player->GetPosition();
	DirectX::XMFLOAT3 playerRot = m_player->GetRotation();
	m_camera->SetPosition(playerPos.x, playerPos.y, playerPos.z);
	m_camera->SetRotation(playerRot.x, playerRot.y, playerRot.z);
	
	m_camera->SetBobbingParameters(
		m_player->IsRunning() ? 18.0f : 14.0f, // bobbingSpeed
		m_player->IsRunning() ? 0.05f : 0.03f, // bobbingAmount
		m_player->IsRunning() ? 10.0f : 7.0f,  // swaySpeed
		m_player->IsRunning() ? 0.08f : 0.05f, // swayAmount
		m_player->IsRunning() ? 9.0f : 7.0f   // rollSpeed
	);

	m_camera->UpdateBobbing(deltaTime, m_player->IsMoving());
	m_camera->Update();

	DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PI / 4.0f, (float)Game::SCREEN_WIDTH / Game::SCREEN_HEIGHT, 0.1f, 1000.0f);

	m_lightManager->Update(deltaTime, m_camera->GetViewMatrix(), projectionMatrix, m_camera->GetPosition(), m_camera->GetRotation());
	m_ui->Update(deltaTime, m_remainingOrbs, m_totalOrbs, m_player->GetStaminaPercentage(), m_enemyRadarTimer > 0.0f);
}


void GameScene::Render()
{
	m_cachedDynamicModels.clear();
	m_cachedDynamicModels.reserve(m_enemies.size() + m_orbs.size() + m_specialOrbs.size());

	for (const auto& enemy : m_enemies) m_cachedDynamicModels.push_back(enemy->GetModel());
	for (const auto& orb : m_orbs) if (Model* orbModel = orb->GetModel()) m_cachedDynamicModels.push_back(orbModel);
	for (const auto& sorb : m_specialOrbs) if (Model* orbModel = sorb->GetModel()) m_cachedDynamicModels.push_back(orbModel);

	if (m_goalOrb)
	{
		if (Model* gModel = m_goalOrb->GetModel()) m_cachedDynamicModels.push_back(gModel);
	}

	// OpenExit()が呼ばれるとGetGateModelはnullptrを返すか、Shutdownされて描画されなくなります
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
	// UIの描画（ミニマップとOrb UIの両方を描画）
	m_ui->Render(m_camera.get(), m_enemies, m_orbs, m_specialOrbs);

	m_graphicsDevice->EndScene();
}