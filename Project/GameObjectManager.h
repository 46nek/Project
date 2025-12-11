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

// 敵、オーブ、ゴールなどの動的オブジェクトを一括管理するクラス
class GameObjectManager {
public:
    GameObjectManager();
    ~GameObjectManager();

    // 初期化：敵やオーブを配置する
    bool Initialize(GraphicsDevice* graphicsDevice, Stage* stage, LightManager* lightManager);

    // 更新：すべてのオブジェクトを動かす
    void Update(float deltaTime, Player* player, Stage* stage, LightManager* lightManager, DirectX::SoundEffect* collectSound);

    // 終了処理
    void Shutdown();

    // 描画に必要なモデルをリストに追加して返す
    void CollectRenderModels(std::vector<Model*>& models);

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
    // 内部処理用の初期化関数
    bool InitializeEnemies(GraphicsDevice* graphicsDevice, Stage* stage);
    bool InitializeOrbs(GraphicsDevice* graphicsDevice, Stage* stage, LightManager* lightManager);
    bool InitializeSpecialOrbs(GraphicsDevice* graphicsDevice, Stage* stage, LightManager* lightManager);
    void SpawnGoal(Stage* stage, LightManager* lightManager);

    // 定数（GameSceneから移動）
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
    float m_enemyRadarTimer;

    // ゲーム内でのみ使う定数などはcpp側に置くか、ここに追加
};