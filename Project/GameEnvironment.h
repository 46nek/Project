#pragma once
#include <memory>
#include <vector>
#include <d3d11.h>
#include <SimpleMath.h>
#include "Stage.h"
#include "LightManager.h"
#include "Renderer.h"
#include "Camera.h"

// 前方宣言 (重要)
class GameObjectManager;

// ステージ、光源、レンダラーなど「ゲームの世界環境」を一括管理するクラス
class GameEnvironment {
public:
    GameEnvironment();
    ~GameEnvironment();

    // 初期化
    bool Initialize(GraphicsDevice* graphicsDevice);

    // 更新（光源の計算など）
    void Update(float deltaTime, const Camera* camera);

    // 描画 (引数に GameObjectManager* を追加)
    void Render(const Camera* camera, const std::vector<Model*>& dynamicModels, float vignetteIntensity, GameObjectManager* gameObjectManager);

    // ゲッター
    Stage* GetStage() const { return m_stage.get(); }
    LightManager* GetLightManager() const { return m_lightManager.get(); }
    Renderer* GetRenderer() const { return m_renderer.get(); }
    const std::vector<Model*>& GetCachedStageModels() const { return m_cachedStageModels; }

private:
    std::unique_ptr<Stage> m_stage;
    std::unique_ptr<LightManager> m_lightManager;
    std::unique_ptr<Renderer> m_renderer;

    GraphicsDevice* m_graphicsDevice = nullptr;

    std::vector<Model*> m_cachedStageModels;
};