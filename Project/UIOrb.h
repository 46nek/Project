#pragma once

#include "GraphicsDevice.h"
#include "Model.h"
#include "Camera.h"
#include <memory>

/**
 * @class UIOrb
 * @brief 画面右上のオーブUIの表示・アニメーションを管理するクラス
 */
class UIOrb
{
public:
    UIOrb();
    ~UIOrb();

    bool Initialize(GraphicsDevice* graphicsDevice);
    void Shutdown();
    void Update(float deltaTime);
    void Render();

private:
    GraphicsDevice* m_graphicsDevice;
    std::unique_ptr<Model> m_orbModel;
    std::unique_ptr<Camera> m_orbCamera;
};