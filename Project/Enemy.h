#pragma once
#include "AStar.h"
#include "Player.h"
#include "ParticleSystem.h"
#include <DirectXMath.h>
#include <d3d11.h>
#include <vector>
#include <memory> 

class GraphicsDevice; 
class Decoy;
class Stage;

/**
 * @brief 謨ｵ繧ｭ繝｣繝ｩ繧ｯ繧ｿ繝ｼ繧ｯ繝ｩ繧ｹ
 */
class Enemy {
public:
    Enemy();
    ~Enemy();

    bool Initialize(ID3D11Device* device, const DirectX::XMFLOAT3& startPosition, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData);
    void Shutdown();
    void Update(float deltaTime, const Player* player, const Stage* stage, const std::vector<Decoy*>& decoys, const std::vector<std::unique_ptr<Enemy>>& enemies);
    
    void Render(GraphicsDevice* graphicsDevice,
        const DirectX::XMMATRIX& viewMatrix,
        const DirectX::XMMATRIX& projectionMatrix,
        const DirectX::XMMATRIX& lightViewMatrix,
        const DirectX::XMMATRIX& lightProjectionMatrix);

    DirectX::XMFLOAT3 GetPosition() const;

private:
    enum State {
        PATROL,
        CHASE,
        SEARCH,
        AMBUSH
    };

    std::unique_ptr<ParticleSystem> m_particleSystem;
    DirectX::XMFLOAT3 m_position;
    float m_speed;
    std::unique_ptr<AStar> m_astar;
    std::vector<DirectX::XMFLOAT2> m_path;
    int m_pathIndex;
    float m_pathCooldown;

    State m_currentState;
    DirectX::XMFLOAT3 m_lastKnownPlayerPos;
    float m_searchTimer;
    float m_ambushTimer;
    DirectX::XMFLOAT3 m_ambushTarget;
    
    // 繧ｹ繧ｿ繝・け讀懃衍逕ｨ
    DirectX::XMFLOAT3 m_prevPosition;
    float m_stuckTimer;

    bool CanSeePlayer(const DirectX::XMFLOAT3& playerPos, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);
};
