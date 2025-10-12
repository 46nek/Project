#pragma once
#include "Scene.h" // 関連ヘッダー
#include <memory>
#include <vector>
#include "Camera.h"
#include "MazeGenerator.h"
#include "Player.h"
#include "LightManager.h"
#include "Model.h"
#include "Renderer.h"
#include "Minimap.h"
#include "PostProcess.h" 

class GameScene : public Scene
{
public:
    GameScene();
    ~GameScene();

    bool Initialize(GraphicsDevice* graphicsDevice, Input* input) override;
    void Shutdown() override;
    void Update(float deltaTime) override;
    void Render() override;

private:
    void HandleInput(float deltaTime);

    // 定数
    static constexpr int MAZE_WIDTH = 21;
    static constexpr int MAZE_HEIGHT = 21;
    static constexpr float PATH_WIDTH = 2.5f;
    static constexpr float WALL_HEIGHT = 5.0f;
    static constexpr float PLAYER_HEIGHT = 4.0f;

    // メンバー変数
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<MazeGenerator> m_mazeGenerator;
    std::unique_ptr<LightManager> m_lightManager;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Minimap> m_minimap;
    std::unique_ptr<Player> m_player;

    std::unique_ptr<PostProcess> m_postProcess;

    std::vector<std::unique_ptr<Model>> m_models;
};