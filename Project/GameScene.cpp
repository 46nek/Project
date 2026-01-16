#include "GameScene.h"
#include "AssetLoader.h"
#include "Game.h"
#include "AssetPaths.h"

extern Game* g_game;

std::unique_ptr<GameScene> GameScene::s_transferInstance = nullptr;

namespace {
    constexpr float MINIMAP_ZOOM_OUT_LEVEL = 2.0f;
    constexpr float CELL_CENTER_OFFSET = 0.5f;     // マスの中心へのオフセット
    constexpr float INITIAL_ROTATION_Y = 180.0f;   // カメラとプレイヤーの初期回転(Y軸)
}

GameScene::GameScene()
    : m_uiFadeTimer(0.0f) {
}

GameScene::~GameScene() {}

bool GameScene::Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine) {
    // ゲーム開始時：カーソルを消して中央固定
    input->SetCursorLock(true);

    m_uiFadeTimer = 0.0f;

    // 前のシーンからのインスタンス引き継ぎ処理
    if (s_transferInstance) {
        m_graphicsDevice = graphicsDevice;
        m_input = input;
        m_audioEngine = audioEngine;

        m_environment = std::move(s_transferInstance->m_environment);

        m_player = std::move(s_transferInstance->m_player);
        m_camera = std::move(s_transferInstance->m_camera);
        m_ui = std::move(s_transferInstance->m_ui);

        // マネージャー引き継ぎ
        m_gameObjectManager = std::move(s_transferInstance->m_gameObjectManager);
        m_cameraDirector = std::move(s_transferInstance->m_cameraDirector);

        m_collectSound = std::move(s_transferInstance->m_collectSound);
        m_walkSoundEffect = std::move(s_transferInstance->m_walkSoundEffect);
        m_runSoundEffect = std::move(s_transferInstance->m_runSoundEffect);

        // キャッシュ変数の引き継ぎ
        m_cachedDynamicModels = s_transferInstance->m_cachedDynamicModels;

        // オープニング中だったら継続
        if (m_cameraDirector && m_cameraDirector->IsOpening()) {
            // カメラ位置などはDirectorが持っているので特に処理不要
        }

        s_transferInstance.reset();

        return true;
    }

    // 新規初期化フロー
    if (!InitializeEnvironment(graphicsDevice, input, audioEngine)) { return false; }
    if (!InitializeUI()) { return false; }
    if (!InitializeGameObjects()) { return false; }
    if (!InitializeAudio()) { return false; }

    return true;
}

bool GameScene::InitializeEnvironment(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine) {
    m_graphicsDevice = graphicsDevice;
    m_input = input;
    m_audioEngine = audioEngine;

    m_environment = std::make_unique<GameEnvironment>();
    if (!m_environment->Initialize(graphicsDevice)) { return false; }

    Stage* stage = m_environment->GetStage();
    std::pair<int, int> startPos = stage->GetStartPosition();
    float pathWidth = stage->GetPathWidth();

    // マスの中心座標を計算
    float startX = (static_cast<float>(startPos.first) + CELL_CENTER_OFFSET) * pathWidth;
    float startZ = (static_cast<float>(startPos.second) + CELL_CENTER_OFFSET) * pathWidth;

    m_player = std::make_unique<Player>();

    m_camera = std::make_shared<Camera>(startX, PLAYER_HEIGHT, startZ);
    m_camera->SetRotation(0.0f, INITIAL_ROTATION_Y, 0.0f);

    m_cameraDirector = std::make_unique<CameraDirector>(m_camera);
    m_cameraDirector->Initialize();

    m_player->Initialize({ startX, PLAYER_HEIGHT, startZ });
    m_player->SetRotation({ 0.0f, INITIAL_ROTATION_Y, 0.0f });

    return true;
}

bool GameScene::InitializeUI() {
    m_ui = std::make_unique<UI>();
    Stage* stage = m_environment->GetStage();
    if (!m_ui->Initialize(m_graphicsDevice, stage->GetMazeData(), stage->GetPathWidth())) { return false; }
    return true;
}

bool GameScene::InitializeGameObjects() {
    m_gameObjectManager = std::make_unique<GameObjectManager>();
    if (!m_gameObjectManager->Initialize(m_graphicsDevice, m_environment->GetStage(), m_environment->GetLightManager())) { return false; }
    return true;
}

bool GameScene::InitializeAudio() {
    try {
        m_collectSound = std::make_unique<DirectX::SoundEffect>(m_audioEngine, AssetPaths::SOUND_ORB_GET);
        m_walkSoundEffect = std::make_unique<DirectX::SoundEffect>(m_audioEngine, AssetPaths::SOUND_WALK);
        m_runSoundEffect = std::make_unique<DirectX::SoundEffect>(m_audioEngine, AssetPaths::SOUND_WALK); // 走る音も同じファイルを使用

        if (m_player) {
            m_player->SetFootstepSounds(m_walkSoundEffect.get(), m_runSoundEffect.get());
        }
    }
    catch (const std::exception& e) {
        MessageBoxA(nullptr, e.what(), "Sound Error", MB_OK);
        return false;
    }
    return true;
}

void GameScene::SetCameraForTitle() {
    if (m_cameraDirector && m_environment) {
        m_cameraDirector->SetCameraForTitle(m_environment->GetStage());
    }
}

void GameScene::UpdateTitleLoop(float deltaTime) {
    if (m_cameraDirector) {
        m_cameraDirector->Update(deltaTime, nullptr, true);
    }

    if (m_environment && m_camera) {
        m_environment->Update(deltaTime, m_camera.get());
    }
}

void GameScene::BeginOpening() {
    if (m_cameraDirector && m_environment) {
        Stage* stage = m_environment->GetStage();
        std::pair<int, int> startPos = stage->GetStartPosition();
        float pathWidth = stage->GetPathWidth();

        float startX = (static_cast<float>(startPos.first) + CELL_CENTER_OFFSET) * pathWidth;
        float startZ = (static_cast<float>(startPos.second) + CELL_CENTER_OFFSET) * pathWidth;

        m_cameraDirector->BeginOpening({ startX, PLAYER_HEIGHT, startZ }, { 0.0f, INITIAL_ROTATION_Y, 0.0f });
    }

    m_uiFadeTimer = 0.0f;
}

void GameScene::Shutdown() {
    if (m_player) { m_player->SetFootstepSounds(nullptr, nullptr); }
    if (m_ui) { m_ui->Shutdown(); }
    if (m_gameObjectManager) { m_gameObjectManager->Shutdown(); }
}

void GameScene::Update(float deltaTime) {
    if (m_cameraDirector && m_cameraDirector->IsOpening()) {
        m_cameraDirector->Update(deltaTime, nullptr, false);

        if (m_environment) {
            m_environment->Update(100.0f, m_camera.get());
        }
        return;
    }

    // --- ESCキーでポーズの切り替え ---
    if (m_input->IsKeyPressed(VK_ESCAPE)) {
        m_isPaused = !m_isPaused;
        m_input->SetCursorLock(!m_isPaused);
        m_input->SetCursorVisible(m_isPaused); // ポーズ中なら表示、再開なら非表示にする

        if (m_isPaused) {
            m_pauseSelectIndex = 0; // 開いた時はSETTINGSを選択
        }
    }

    // --- ポーズ中のロジック ---
    if (m_isPaused) {
        // 1. マウス座標の変換 (1280x720の論理座標へ)
        int rawMx, rawMy;
        m_input->GetMousePosition(rawMx, rawMy);
        HWND hwnd = g_game->GetWindow()->GetHwnd();
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        float actualWidth = static_cast<float>(clientRect.right - clientRect.left);
        float actualHeight = static_cast<float>(clientRect.bottom - clientRect.top);
        if (actualWidth <= 0) actualWidth = 1.0f;
        if (actualHeight <= 0) actualHeight = 1.0f;
        float mx = rawMx * (static_cast<float>(Game::SCREEN_WIDTH) / actualWidth);
        float my = rawMy * (static_cast<float>(Game::SCREEN_HEIGHT) / actualHeight);

        // 2. UIの更新 (AUDIO削除後の新インデックス体系で呼び出し)
        m_ui->UpdatePauseMenu(m_pauseSelectIndex, mx, my, m_input->IsKeyPressed(VK_LBUTTON), m_input->IsKeyDown(VK_LBUTTON), Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT);

        // 3. キーボード操作 (最大値を 5 に変更)
        if (m_input->IsKeyPressed(VK_UP)) {
            m_pauseSelectIndex = (m_pauseSelectIndex > 0) ? m_pauseSelectIndex - 1 : 5;
        }
        if (m_input->IsKeyPressed(VK_DOWN)) {
            m_pauseSelectIndex = (m_pauseSelectIndex < 5) ? m_pauseSelectIndex + 1 : 0;
        }

        // 4. 決定操作 (RETURN: 4, EXIT: 5)
        bool isDecisionMade = m_input->IsKeyPressed(VK_RETURN) || (m_input->IsKeyPressed(VK_LBUTTON) && (m_pauseSelectIndex == 4 || m_pauseSelectIndex == 5));

        if (isDecisionMade) {
            if (m_pauseSelectIndex == 4) { // RETURN TO GAME
                m_isPaused = false;
                m_input->SetCursorLock(true);
                m_input->SetCursorVisible(false); // カーソルを隠す
            }
            else if (m_pauseSelectIndex == 5) { // BACK TO TITLE
                m_nextScene = SceneState::Title;
            }
        }
        return;
    }

    if (!m_gameObjectManager) { return; }

    if (m_input->IsKeyPressed(VK_ESCAPE)) {
        m_input->SetCursorLock(false);
    }
    else if (m_input->IsKeyPressed(VK_LBUTTON) || (GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
        m_input->SetCursorLock(true);
    }

    if (m_uiFadeTimer < UI_FADE_DURATION) {
        m_uiFadeTimer += deltaTime;
        if (m_uiFadeTimer > UI_FADE_DURATION) { m_uiFadeTimer = UI_FADE_DURATION; }
    }

    int mouseX, mouseY;
    m_input->GetMouseDelta(mouseX, mouseY);
    m_player->Turn(mouseX, mouseY, deltaTime);

    m_player->Update(deltaTime, m_input, m_environment->GetStage()->GetMazeData(), m_environment->GetStage()->GetPathWidth());

    if (m_cameraDirector) {
        m_cameraDirector->Update(deltaTime, m_player.get(), false);
    }

    m_gameObjectManager->Update(deltaTime, m_player.get(), m_environment->GetStage(), m_environment->GetLightManager(), m_collectSound.get());

    if (m_gameObjectManager->IsEscapeMode()) {
        DirectX::XMFLOAT3 pPos = m_player->GetPosition();
        if (pPos.z < m_environment->GetStage()->GetPathWidth() * 0.8f) {
            m_nextScene = SceneState::Result;
        }
    }

    m_environment->Update(deltaTime, m_camera.get());

    if (m_gameObjectManager->CheckAndResetZoomRequest()) {
        m_ui->SetMinimapZoom(MINIMAP_ZOOM_OUT_LEVEL);
    }

    m_ui->Update(deltaTime,
        m_gameObjectManager->GetRemainingOrbs(),
        m_gameObjectManager->GetTotalOrbs(),
        m_player->GetSkillDurationTimer(), // 残り走行時間
        m_player->GetSkillCooldownTimer(), // クールダウン時間
        m_player->IsSkillActive(),         // 走行中かどうか
        m_gameObjectManager->GetEnemyRadarTimer() > 0.0f);
}

void GameScene::RenderStageOnly() {
    m_cachedDynamicModels.clear();

    if (m_gameObjectManager) {
        m_gameObjectManager->CollectRenderModels(m_cachedDynamicModels);
    }

    if (m_environment) {
        if (Model* gate = m_environment->GetStage()->GetGateModel()) {
            m_cachedDynamicModels.push_back(gate);
        }
    }

    float vignette = 0.0f;
    if (m_cameraDirector) {
        vignette = m_cameraDirector->GetVignetteIntensity();
    }

    if (m_environment) {
        m_environment->Render(m_camera.get(), m_cachedDynamicModels, vignette, m_gameObjectManager.get());
    }
}

void GameScene::Render() {
    if (!m_gameObjectManager) { return; }

    // 3D空間（ステージやキャラクター）の描画
    RenderStageOnly();

    if (m_ui) {
        if (m_isPaused) {
            m_ui->RenderPauseMenu(m_pauseSelectIndex, 1280, 720);
        }
        else {
            float uiAlpha = 1.0f;
            if (m_cameraDirector && m_cameraDirector->IsOpening()) {
                uiAlpha = 0.0f;
            }
            else {
                uiAlpha = m_uiFadeTimer / UI_FADE_DURATION;
                if (uiAlpha > 1.0f) { uiAlpha = 1.0f; }
            }

            m_ui->Render(m_camera.get(),
                m_gameObjectManager->GetEnemies(),
                m_gameObjectManager->GetOrbs(),
                m_gameObjectManager->GetSpecialOrbs(),
                uiAlpha);
        }
    }

    m_graphicsDevice->EndScene();
}