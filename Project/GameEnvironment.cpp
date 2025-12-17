#include "GameEnvironment.h"
#include "Game.h" 
#include "GameObjectManager.h" 

GameEnvironment::GameEnvironment() {}
GameEnvironment::~GameEnvironment() {}

bool GameEnvironment::Initialize(GraphicsDevice* graphicsDevice) {
    // 追加: デバイスを保持しておく
    m_graphicsDevice = graphicsDevice;

    // ステージ生成
    m_stage = std::make_unique<Stage>();
    if (!m_stage->Initialize(graphicsDevice)) { return false; }

    // ライト管理
    m_lightManager = std::make_unique<LightManager>();
    m_lightManager->Initialize(m_stage->GetMazeData(), m_stage->GetPathWidth(), Stage::WALL_HEIGHT);

    // レンダラー
    m_renderer = std::make_unique<Renderer>(graphicsDevice);

    // ステージモデルのキャッシュ
    m_cachedStageModels.clear();
    // ここでモデルへのポインタをキャッシュ
    for (const auto& model : m_stage->GetModels()) {
        if (model) {
            m_cachedStageModels.push_back(model.get());
        }
    }

    return true;
}

void GameEnvironment::Update(float deltaTime, const Camera* camera) {
    if (m_lightManager && camera) {
        DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
            DirectX::XM_PI / 4.0f,
            (float)Game::SCREEN_WIDTH / Game::SCREEN_HEIGHT,
            0.1f, 1000.0f
        );

        m_lightManager->Update(deltaTime, camera->GetViewMatrix(), projectionMatrix, camera->GetPosition(), camera->GetRotation());
    }
}

// 引数を修正
void GameEnvironment::Render(const Camera* camera, const std::vector<Model*>& dynamicModels, float vignetteIntensity, GameObjectManager* gameObjectManager) {
    if (m_renderer && m_stage) {
        // 1. 通常モデルの描画
        m_renderer->RenderSceneToTexture(
            m_cachedStageModels,
            dynamicModels,
            camera,
            m_lightManager.get(),
            m_stage->GetMazeData(),
            m_stage->GetPathWidth()
        );

        // 2. 敵（パーティクル）の描画
        // シーン描画の後、ポストプロセスの前に実行する
        if (gameObjectManager && m_graphicsDevice && m_lightManager) {
            gameObjectManager->RenderEnemies(m_graphicsDevice, const_cast<Camera*>(camera), m_lightManager.get());
        }

        // 3. 仕上げ（ポストプロセス）
        m_renderer->RenderFinalPass(camera, vignetteIntensity);
    }
}