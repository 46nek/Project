#include "GameEnvironment.h"
#include "Game.h" 
#include "GameObjectManager.h" 

GameEnvironment::GameEnvironment() {}
GameEnvironment::~GameEnvironment() {}

bool GameEnvironment::Initialize(GraphicsDevice* graphicsDevice) {
    m_graphicsDevice = graphicsDevice;

    // 繧ｹ繝・・繧ｸ逕滓・
    m_stage = std::make_unique<Stage>();
    if (!m_stage->Initialize(graphicsDevice)) { return false; }

    // 繝ｩ繧､繝育ｮ｡逅・
    m_lightManager = std::make_unique<LightManager>();
    m_lightManager->Initialize(m_stage->GetMazeData(), m_stage->GetPathWidth(), Stage::WALL_HEIGHT);

    // 繝ｬ繝ｳ繝繝ｩ繝ｼ
    m_renderer = std::make_unique<Renderer>(graphicsDevice);

    // 繧ｹ繝・・繧ｸ繝｢繝・Ν縺ｮ繧ｭ繝｣繝・す繝･
    m_cachedStageModels.clear();
    // 縺薙％縺ｧ繝｢繝・Ν縺ｸ縺ｮ繝昴う繝ｳ繧ｿ繧偵く繝｣繝・す繝･
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

// 蠑墓焚繧剃ｿｮ豁｣
void GameEnvironment::Render(const Camera* camera, const std::vector<Model*>& dynamicModels, float vignetteIntensity, GameObjectManager* gameObjectManager) {
    if (m_renderer && m_stage) {
        // 1. 騾壼ｸｸ繝｢繝・Ν縺ｮ謠冗判
        m_renderer->RenderSceneToTexture(
            m_cachedStageModels,
            dynamicModels,
            camera,
            m_lightManager.get(),
            m_stage->GetMazeData(),
            m_stage->GetPathWidth()
        );

        // 2. 謨ｵ・医ヱ繝ｼ繝・ぅ繧ｯ繝ｫ・峨・謠冗判
        // 繧ｷ繝ｼ繝ｳ謠冗判縺ｮ蠕後√・繧ｹ繝医・繝ｭ繧ｻ繧ｹ縺ｮ蜑阪↓螳溯｡後☆繧・
        if (gameObjectManager && m_graphicsDevice && m_lightManager) {
            gameObjectManager->RenderEnemies(m_graphicsDevice, const_cast<Camera*>(camera), m_lightManager.get());
        }

        // 3. 莉穂ｸ翫￡・医・繧ｹ繝医・繝ｭ繧ｻ繧ｹ・・
        m_renderer->RenderFinalPass(camera, vignetteIntensity);
    }
}
