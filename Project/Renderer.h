#pragma once
#include <vector>
#include <memory>
#include "GraphicsDevice.h"
#include "Model.h"
#include "LightManager.h"
#include "Camera.h"

/**
 * @brief 3Dシーンの描画処理を統括するクラス
 */
class Renderer
{
public:
    Renderer(GraphicsDevice* graphicsDevice);
    ~Renderer();
    /**
        * @brief シャドウマップ生成パス
        */
    void RenderDepthPass(const std::vector<std::unique_ptr<Model>>& models, LightManager* lightManager);

    /**
     * @brief 通常の描画パス
     */
    void RenderMainPass(const std::vector<std::unique_ptr<Model>>& models, const Camera* camera, LightManager* lightManager);
    // --- ▲▲▲ ここまで修正 ▲▲▲ ---

private:
    GraphicsDevice* m_graphicsDevice;
};