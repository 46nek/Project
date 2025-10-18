#pragma once
#include "Scene.h"
#include <memory>
#include <vector>
#include "Camera.h"
#include "Player.h"
#include "LightManager.h"
#include "Renderer.h"
#include "Stage.h"
#include "Enemy.h"
#include "Orb.h"
#include "UI.h"

class GameScene : public Scene
{
public:
    GameScene();
    ~GameScene();

    bool Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine) override;
    bool InitializePhase1(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine);
    bool InitializePhase2();
    bool InitializePhase3();
    bool InitializePhase4();
    bool InitializePhase5();
    void Shutdown() override;
    void Update(float deltaTime) override;
    void Render() override;

private:
    bool InitializeEnemies();
    bool InitializeOrbs();
    bool InitializeSpecialOrbs();

    static constexpr float PLAYER_HEIGHT = 4.0f;
    static constexpr int NUM_ENEMIES = 2;
    static constexpr int NUM_ORBS = 50;

    std::unique_ptr<Stage> m_stage;
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<LightManager> m_lightManager;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Player> m_player;
    std::vector<std::unique_ptr<Enemy>> m_enemies;
    std::vector<std::unique_ptr<Orb>> m_orbs; 
    std::vector<std::unique_ptr<Orb>> m_specialOrbs; // 特殊オーブのリスト
    float m_enemyRadarTimer; // 敵レーダーの効果時間タイマー
    std::unique_ptr<UI> m_ui;

    std::unique_ptr<DirectX::SoundEffect> m_collectSound; 
    
    float m_vignetteIntensity;
};