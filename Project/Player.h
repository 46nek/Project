#pragma once
#include "Input.h"
#include "MazeGenerator.h"
#include <DirectXMath.h>
#include <vector>
#include <algorithm>
#include <memory>

// 蜑肴婿螳｣險
namespace DirectX {
    class SoundEffect;
    class SoundEffectInstance;
}

/**
 * @brief 繝励Ξ繧､繝､繝ｼ繧ｯ繝ｩ繧ｹ
 */
class Player {
public:
    Player();
    ~Player();

    void Initialize(const DirectX::XMFLOAT3& startPosition);
    void Update(float deltaTime, Input* input, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);

    void UpdateSkill(float deltaTime, Input* input);

    void UpdateMovement(float deltaTime, Input* input, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);
    void UpdateAudio(float deltaTime);
    void Turn(int mouseX, int mouseY, float deltaTime);

    void SetRotation(const DirectX::XMFLOAT3& rotation);
    void SetFootstepSounds(DirectX::SoundEffect* walkSound, DirectX::SoundEffect* runSound);
    void SetHeldOrbCount(int count) { m_heldOrbCount = count; }

    DirectX::XMFLOAT3 GetPosition() const { return m_position; }
    DirectX::XMFLOAT3 GetRotation() const { return m_rotation; }
    bool IsMoving() const { return m_isMoving; }

    // 繧ｹ繧ｭ繝ｫ迥ｶ諷九・蜿門ｾ・
    bool IsSkillActive() const { return m_isRunning; }
    float GetSkillDurationTimer() const { return m_skillDurationTimer; }
    float GetSkillCooldownTimer() const { return m_skillCoolDownTimer; }
    int GetHeldOrbCount() const { return m_heldOrbCount; }
    bool IsDecoyRequested() const { return m_isDecoyRequested; }
    void ResetDecoyRequest() { m_isDecoyRequested = false; }

private:
    static constexpr float COLLISION_RADIUS = 0.45f;

    // 繧ｹ繧ｭ繝ｫ螳壽焚
    static constexpr float RUN_SKILL_DURATION = 3.0f;  // 3遘帝俣襍ｰ繧後ｋ
    static constexpr float RUN_SKILL_COOLDOWN = 10.0f; // 蜀堺ｽｿ逕ｨ縺ｾ縺ｧ10遘・

    bool IsCollidingWithWall(const DirectX::XMFLOAT3& position, float radius, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);

    DirectX::XMFLOAT3 m_position;
    DirectX::XMFLOAT3 m_rotation;

    float m_moveSpeed;
    float m_runSpeed;
    float m_rotationSpeed;
    bool m_isMoving;
    bool m_isRunning;

    // 繧ｹ繧ｭ繝ｫ逕ｨ繧ｿ繧､繝槭・
    float m_skillDurationTimer; // 襍ｰ繧後ｋ谿九ｊ譎る俣
    float m_skillCoolDownTimer; // 蜀堺ｽｿ逕ｨ縺ｾ縺ｧ縺ｮ譎る俣

    DirectX::SoundEffect* m_walkSound;
    DirectX::SoundEffect* m_runSound;
    std::unique_ptr<DirectX::SoundEffectInstance> m_walkInstance;
    std::unique_ptr<DirectX::SoundEffectInstance> m_runInstance;

    float m_stepTimer;
    float m_walkInterval;
    float m_runInterval;

    int m_heldOrbCount = 0;
    bool m_isDecoyRequested = false;
};
