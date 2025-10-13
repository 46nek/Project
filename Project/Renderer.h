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

       // --- 新しい描画フロー ---
        // 1. シーンをテクスチャにレンダリングする
        void RenderSceneToTexture(const std::vector<std::unique_ptr<Model>>&models, const Camera * camera, LightManager * lightManager);
    // 2. レンダリングされたテクスチャにポストプロセスを適用し、画面に描画する
    void RenderFinalPass(const Camera * camera);

private:
    // シャドウマップ生成パス
    void RenderDepthPass(const std::vector<std::unique_ptr<Model>>& models, LightManager* lightManager);
    // 通常の描画パス
    void RenderMainPass(const std::vector<std::unique_ptr<Model>>& models, const Camera* camera, LightManager* lightManager);

    GraphicsDevice* m_graphicsDevice;
};