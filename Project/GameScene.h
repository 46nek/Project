#pragma once
#include "Scene.h"
#include <memory>
#include <vector>
#include "Camera.h"
#include "Player.h"
#include "LightManager.h"
#include "Renderer.h"
#include "Minimap.h"
#include "Stage.h" 

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
    // ’è”
    static constexpr float PLAYER_HEIGHT = 4.0f;

    // ƒƒ“ƒo[•Ï”
    std::unique_ptr<Stage> m_stage; 
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<LightManager> m_lightManager;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Minimap> m_minimap;
    std::unique_ptr<Player> m_player;
};