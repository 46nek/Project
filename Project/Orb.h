#pragma once
#include "Model.h"
#include "Player.h"
#include "LightManager.h"
#include "Audio.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

/**
 * @brief 繧ｪ繝ｼ繝悶・遞ｮ鬘槫ｮ夂ｾｩ
 */
enum class OrbType {
    Normal,
    MinimapZoomOut,
    EnemyRadar,
    Goal
};

/**
 * @brief 繧ｲ繝ｼ繝蜀・・蜿朱寔繧｢繧､繝・Β・医が繝ｼ繝厄ｼ峨け繝ｩ繧ｹ
 */
class Orb {
public:
    Orb();
    ~Orb();

    bool Initialize(ID3D11Device* device, const DirectX::XMFLOAT3& position, int lightIndex, OrbType type = OrbType::Normal);
    void Shutdown();
    bool Update(float deltaTime, Player* player, LightManager* lightManager, DirectX::SoundEffect* collectSound);
    void FollowPlayer(float deltaTime, const DirectX::XMFLOAT3& targetPos, int index);
    Model* GetModel();
    
    bool IsCollected() const;
    bool IsDelivered() const { return m_isDelivered; }

    DirectX::XMFLOAT3 GetPosition() const;
    OrbType GetType() const; 

private:
    std::unique_ptr<Model> m_model;
    DirectX::XMFLOAT3 m_position;

    bool m_isCollected;
    bool m_isDelivered = false;
    int m_lightIndex;
    float m_animationTimer;
    OrbType m_type;
};
