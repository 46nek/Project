#pragma once
#include "GraphicsDevice.h"
#include "Input.h"

// シーンの状態
enum class SceneState
{
    None,
    Title,
    Game,
};

/**
 * @brief 各シーンの基底となる抽象クラス
 */
class Scene
{
public:
    Scene() : m_graphicsDevice(nullptr), m_input(nullptr), m_nextScene(SceneState::None) {}
    virtual ~Scene() = default;

    virtual bool Initialize(GraphicsDevice* graphicsDevice, Input* input) = 0;
    virtual void Shutdown() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;

    SceneState GetNextScene() const { return m_nextScene; }

protected:
    GraphicsDevice* m_graphicsDevice;
    Input* m_input;
    SceneState m_nextScene;
};