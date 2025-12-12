#pragma once
#include <memory>
#include <vector>
#include <d3d11.h>
#include <SimpleMath.h>
#include "Stage.h"
#include "LightManager.h"
#include "Renderer.h"
#include "Camera.h"

// ステージ、光源、レンダラーなど「ゲームの世界環境」を一括管理するクラス
class GameEnvironment {
public:
    GameEnvironment();
    ~GameEnvironment();

    // 初期化
    bool Initialize(GraphicsDevice* graphicsDevice);

    // 更新（光源の計算など）
    void Update(float deltaTime, const Camera* camera);

    // 描画
    void Render(const Camera* camera, const std::vector<Model*>& dynamicModels, float vignetteIntensity);

    // ゲッター（GameSceneやDirectorが必要とする情報）
    Stage* GetStage() const { return m_stage.get(); }
    LightManager* GetLightManager() const { return m_lightManager.get(); }
    Renderer* GetRenderer() const { return m_renderer.get(); }
    const std::vector<Model*>& GetCachedStageModels() const { return m_cachedStageModels; }

private:
    std::unique_ptr<Stage> m_stage;
    std::unique_ptr<LightManager> m_lightManager;
    std::unique_ptr<Renderer> m_renderer;

    std::vector<Model*> m_cachedStageModels;
};