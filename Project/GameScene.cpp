#include "GameScene.h"
#include "AssetLoader.h"
#include "Game.h"
#include "AssetPaths.h"

std::unique_ptr<GameScene> GameScene::s_transferInstance = nullptr;

// グローバル変数のGameインスタンスを参照
extern Game* g_game;

GameScene::GameScene()
	: m_uiFadeTimer(0.0f)
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

		// マネージャー引き継ぎ
		m_gameObjectManager = std::move(s_transferInstance->m_gameObjectManager);
		m_cameraDirector = std::move(s_transferInstance->m_cameraDirector);

		m_collectSound = std::move(s_transferInstance->m_collectSound);
		m_walkSoundEffect = std::move(s_transferInstance->m_walkSoundEffect);
		m_runSoundEffect = std::move(s_transferInstance->m_runSoundEffect);

		m_cachedStageModels = s_transferInstance->m_cachedStageModels;
		m_cachedDynamicModels = s_transferInstance->m_cachedDynamicModels;

		// オープニング中だったら継続
		if (m_cameraDirector && m_cameraDirector->IsOpening())
		{
			// カメラ位置などはDirectorが持っているので特に処理不要
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

	// ★ Cameraをshared_ptrで作成
	m_camera = std::make_shared<Camera>(startX, PLAYER_HEIGHT, startZ);
	m_camera->SetRotation(0.0f, 180.0f, 0.0f);

	// ★ CameraDirectorの作成と初期化
	m_cameraDirector = std::make_unique<CameraDirector>(m_camera);
	m_cameraDirector->Initialize();

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
	m_gameObjectManager = std::make_unique<GameObjectManager>();
	if (!m_gameObjectManager->Initialize(m_graphicsDevice, m_stage.get(), m_lightManager.get())) return false;
	return true;
}

bool GameScene::InitializePhase4()
{
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
	// ★ Directorに委譲
	if (m_cameraDirector && m_stage)
	{
		m_cameraDirector->SetCameraForTitle(m_stage.get());
	}
}

void GameScene::UpdateTitleLoop(float deltaTime)
{
	// ★ Directorに委譲
	if (m_cameraDirector)
	{
		m_cameraDirector->Update(deltaTime, nullptr, true); // isTitle = true
	}

	if (m_lightManager && m_camera)
	{
		DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PI / 4.0f, (float)Game::SCREEN_WIDTH / Game::SCREEN_HEIGHT, 0.1f, 1000.0f);
		m_lightManager->Update(deltaTime, m_camera->GetViewMatrix(), projectionMatrix, m_camera->GetPosition(), m_camera->GetRotation());
	}
}

void GameScene::BeginOpening()
{
	// ★ Directorに委譲
	if (m_cameraDirector && m_stage)
	{
		std::pair<int, int> startPos = m_stage->GetStartPosition();
		float pathWidth = m_stage->GetPathWidth();
		float startX = (static_cast<float>(startPos.first) + 0.5f) * pathWidth;
		float startZ = (static_cast<float>(startPos.second) + 0.5f) * pathWidth;

		m_cameraDirector->BeginOpening({ startX, PLAYER_HEIGHT, startZ }, { 0.0f, 180.0f, 0.0f });
	}

	m_uiFadeTimer = 0.0f;
}

void GameScene::Shutdown()
{
	if (m_player) m_player->SetFootstepSounds(nullptr, nullptr);
	if (m_ui) m_ui->Shutdown();
	if (m_gameObjectManager) m_gameObjectManager->Shutdown();
	if (m_stage) m_stage->Shutdown();
}

void GameScene::Update(float deltaTime)
{
	// ★ Directorがオープニング中かチェック
	if (m_cameraDirector && m_cameraDirector->IsOpening())
	{
		// オープニング中のカメラ更新
		m_cameraDirector->Update(deltaTime, nullptr, false);

		if (m_lightManager) {
			// 光源更新（高速回転など必要なら）
			DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PI / 4.0f, (float)Game::SCREEN_WIDTH / Game::SCREEN_HEIGHT, 0.1f, 1000.0f);
			m_lightManager->Update(100.0f, m_camera->GetViewMatrix(), projectionMatrix, m_camera->GetPosition(), m_camera->GetRotation());
		}
		return;
	}

	// プレイ中の更新
	if (!m_gameObjectManager) return;

	if (m_input->IsKeyPressed(VK_ESCAPE))
	{
		m_input->SetCursorLock(false);
	}
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

	// ★ Directorによるカメラ更新（ボビング、FOV、ビニエット含む）
	if (m_cameraDirector) {
		m_cameraDirector->Update(deltaTime, m_player.get(), false);
	}

	m_gameObjectManager->Update(deltaTime, m_player.get(), m_stage.get(), m_lightManager.get(), m_collectSound.get());

	if (m_gameObjectManager->IsEscapeMode())
	{
		DirectX::XMFLOAT3 pPos = m_player->GetPosition();
		if (pPos.z < m_stage->GetPathWidth() * 0.8f)
		{
			m_nextScene = SceneState::Result;
		}
	}

	DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PI / 4.0f, (float)Game::SCREEN_WIDTH / Game::SCREEN_HEIGHT, 0.1f, 1000.0f);
	m_lightManager->Update(deltaTime, m_camera->GetViewMatrix(), projectionMatrix, m_camera->GetPosition(), m_camera->GetRotation());

	m_ui->Update(deltaTime,
		m_gameObjectManager->GetRemainingOrbs(),
		m_gameObjectManager->GetTotalOrbs(),
		m_player->GetStaminaPercentage(),
		m_gameObjectManager->GetEnemyRadarTimer() > 0.0f);
}

void GameScene::RenderStageOnly()
{
	m_cachedDynamicModels.clear();

	if (m_gameObjectManager) {
		m_gameObjectManager->CollectRenderModels(m_cachedDynamicModels);
	}

	if (m_stage) {
		if (Model* gate = m_stage->GetGateModel())
		{
			m_cachedDynamicModels.push_back(gate);
		}
	}

	// ★ ビネット強度はDirectorから取得
	float vignette = 0.0f;
	if (m_cameraDirector) {
		vignette = m_cameraDirector->GetVignetteIntensity();
	}

	if (m_renderer && m_stage) {
		m_renderer->RenderSceneToTexture(
			m_cachedStageModels,
			m_cachedDynamicModels,
			m_camera.get(),
			m_lightManager.get(),
			m_stage->GetMazeData(),
			m_stage->GetPathWidth()
		);
		m_renderer->RenderFinalPass(m_camera.get(), vignette);
	}
}

void GameScene::Render()
{
	if (!m_gameObjectManager) return; // ここはゲームプレイ中のみ呼ぶ前提なら弾いてOK

	RenderStageOnly();

	if (m_ui)
	{
		float uiAlpha = 1.0f;
		if (m_cameraDirector && m_cameraDirector->IsOpening())
		{
			uiAlpha = 0.0f;
		}
		else
		{
			uiAlpha = m_uiFadeTimer / UI_FADE_DURATION;
			if (uiAlpha > 1.0f) uiAlpha = 1.0f;
		}

		m_ui->Render(m_camera.get(),
			m_gameObjectManager->GetEnemies(),
			m_gameObjectManager->GetOrbs(),
			m_gameObjectManager->GetSpecialOrbs(),
			uiAlpha);
	}

	m_graphicsDevice->EndScene();
}