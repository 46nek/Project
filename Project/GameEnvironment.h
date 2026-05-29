#pragma once
#include <memory>
#include <vector>
#include <d3d11.h>
#include <SimpleMath.h>
#include "Stage.h"
#include "LightManager.h"
#include "Renderer.h"
#include "Camera.h"

class GameObjectManager;

// ステージ、光源、レンダラーなどゲームの世界環境を一括管理するクラス
class GameEnvironment {
public:
    GameEnvironment();
    ~GameEnvironment();

    bool Initialize(GraphicsDevice* graphicsDevice);
    void Update(float deltaTime, const Camera* camera);
    void Render(const Camera* camera, const std::vector<Model*>& dynamicModels, float vignetteIntensity, GameObjectManager* gameObjectManager);

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
