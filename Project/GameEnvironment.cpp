#include "GameEnvironment.h"
#include "Game.h" // 画面サイズ定数などのため

GameEnvironment::GameEnvironment() {}
GameEnvironment::~GameEnvironment() {}

bool GameEnvironment::Initialize(GraphicsDevice* graphicsDevice) {
    // ステージ生成
    m_stage = std::make_unique<Stage>();
    if (!m_stage->Initialize(graphicsDevice)) { return false; }

    // ライト管理
    m_lightManager = std::make_unique<LightManager>();
    // ステージ情報からライト初期化
    m_lightManager->Initialize(m_stage->GetMazeData(), m_stage->GetPathWidth(), Stage::WALL_HEIGHT);

    // レンダラー
    m_renderer = std::make_unique<Renderer>(graphicsDevice);

    // ステージモデルのキャッシュ
    m_cachedStageModels.clear();
    m_cachedStageModels.reserve(m_stage->GetModels().size());
    for (const auto& model : m_stage->GetModels()) {
        m_cachedStageModels.push_back(model.get());
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

void GameEnvironment::Render(const Camera* camera, const std::vector<Model*>& dynamicModels, float vignetteIntensity) {
    if (m_renderer && m_stage) {
        m_renderer->RenderSceneToTexture(
            m_cachedStageModels,
            dynamicModels,
            camera,
            m_lightManager.get(),
            m_stage->GetMazeData(),
            m_stage->GetPathWidth()
        );
        m_renderer->RenderFinalPass(camera, vignetteIntensity);
    }
}