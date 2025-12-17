#pragma once
#include <vector>
#include <memory>
#include <d3d11.h>
#include <SimpleMath.h>
#include "Enemy.h"
#include "Orb.h"
#include "Stage.h"
#include "LightManager.h"
#include "Player.h"
#include "GraphicsDevice.h"

// 前方宣言 (重要)
class Camera;

class GameObjectManager {
public:
    GameObjectManager();
    ~GameObjectManager();

    bool Initialize(GraphicsDevice* graphicsDevice, Stage* stage, LightManager* lightManager);
    void Update(float deltaTime, Player* player, Stage* stage, LightManager* lightManager, DirectX::SoundEffect* collectSound);
    void Shutdown();

    void CollectRenderModels(std::vector<Model*>& models);
    bool CheckAndResetZoomRequest();

    // ▼▼▼ 追加: 敵のパーティクルを描画する関数 ▼▼▼
    void RenderEnemies(GraphicsDevice* graphicsDevice, Camera* camera, LightManager* lightManager);

    // ゲッター
    int GetRemainingOrbs() const { return m_remainingOrbs; }
    int GetTotalOrbs() const { return m_totalOrbs; }
    bool IsGoalSpawned() const { return m_goalSpawned; }
    bool IsEscapeMode() const { return m_escapeMode; }
    float GetEnemyRadarTimer() const { return m_enemyRadarTimer; }
    const std::vector<std::unique_ptr<Enemy>>& GetEnemies() const { return m_enemies; }
    const std::vector<std::unique_ptr<Orb>>& GetOrbs() const { return m_orbs; }
    const std::vector<std::unique_ptr<Orb>>& GetSpecialOrbs() const { return m_specialOrbs; }

private:
    GraphicsDevice* m_graphicsDevice = nullptr;
    bool InitializeEnemies(GraphicsDevice* graphicsDevice, Stage* stage);
    bool InitializeOrbs(GraphicsDevice* graphicsDevice, Stage* stage, LightManager* lightManager);
    bool InitializeSpecialOrbs(GraphicsDevice* graphicsDevice, Stage* stage, LightManager* lightManager);
    void SpawnGoal(Stage* stage, LightManager* lightManager);

    static constexpr int NUM_ENEMIES = 2;
    static constexpr int NUM_ORBS = 20;

    std::vector<std::unique_ptr<Enemy>> m_enemies;
    std::vector<std::unique_ptr<Orb>> m_orbs;
    std::vector<std::unique_ptr<Orb>> m_specialOrbs;
    std::unique_ptr<Orb> m_goalOrb;

    int m_remainingOrbs;
    int m_totalOrbs;
    bool m_goalSpawned;
    bool m_escapeMode;
    bool m_requestZoomOut;
    float m_enemyRadarTimer;
};