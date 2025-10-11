#pragma once
#include <memory>
#include <vector>
#include "Scene.h"
#include "Camera.h"
#include "MazeGenerator.h"
#include "LightManager.h"
#include "Model.h"
#include "Renderer.h"
#include "Minimap.h" 

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

    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<MazeGenerator> m_mazeGenerator;
    std::unique_ptr<LightManager> m_lightManager;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Minimap> m_minimap;

    std::vector<std::unique_ptr<Model>> m_models;
};