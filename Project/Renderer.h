#pragma once
#include <vector>
#include <memory>
#include "GraphicsDevice.h"
#include "Model.h"
#include "LightManager.h"
#include "Camera.h"
#include "PostProcess.h"

/**
 * @brief 3Dシーンの描画処理を統括するクラス
 */
class Renderer
{
public:
    Renderer(GraphicsDevice* graphicsDevice);
    ~Renderer();

    /**
     * @brief シーン全体を描画
     * @param models 描画するモデルのリスト
     * @param camera カメラ
     * @param lightManager ライトマネージャー
     */
    void RenderScene(const std::vector<std::unique_ptr<Model>>& models, const Camera* camera, LightManager* lightManager);

private:
    // シャドウマップ生成パス
    void RenderDepthPass(const std::vector<std::unique_ptr<Model>>& models, LightManager* lightManager);
    // 通常の描画パス
    void RenderMainPass(const std::vector<std::unique_ptr<Model>>& models, const Camera* camera, LightManager* lightManager);

    GraphicsDevice* m_graphicsDevice;
};