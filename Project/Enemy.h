#pragma once
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include "Model.h"
#include "AStar.h"
#include "Player.h"
#include <d3d11.h>

class Enemy
{
public:
    // --- ここから追加 ---
    enum class AIState
    {
        Wandering, // エリアを徘徊している状態
        Chasing    // プレイヤーを追跡している状態
    };
    // --- 追加ここまで ---

    Enemy();
    ~Enemy();

    bool Initialize(ID3D11Device* device, const DirectX::XMFLOAT3& startPosition, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData); // 引数を追加
    void Shutdown();
    void Update(float deltaTime, const Player* player, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);
    Model* GetModel();
    DirectX::XMFLOAT3 GetPosition() const;

private:
    // --- ここから追加 ---
    bool CanSeePlayer(const Player* player, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);
    void Wander(float deltaTime, float pathWidth);
    // --- 追加ここまで ---

    std::unique_ptr<Model> m_model;
    DirectX::XMFLOAT3 m_position;
    float m_speed;
    std::unique_ptr<AStar> m_astar;
    std::vector<DirectX::XMFLOAT2> m_path;

    int m_pathIndex;
    float m_pathCooldown;

    AIState m_currentState;        // 現在のAIの状態
    float m_stateTimer;            // 状態を管理するためのタイマー
    DirectX::XMFLOAT3 m_wanderTarget; // 徘徊モードの目標地点
};
