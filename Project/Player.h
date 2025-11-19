// Project/Player.h

#pragma once
#include <DirectXMath.h>
#include <vector>
#include "Input.h"
#include "MazeGenerator.h"
#include <algorithm> 

namespace DirectX {
	class SoundEffect;
}

class Player
{
public:
	Player();

	void Initialize(const DirectX::XMFLOAT3& startPosition);
	void Update(float deltaTime, Input* input, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);
	void Turn(int mouseX, int mouseY, float deltaTime);

	void SetFootstepSounds(DirectX::SoundEffect* walkSound, DirectX::SoundEffect* runSound);

	DirectX::XMFLOAT3 GetPosition() const { return m_position; }
	DirectX::XMFLOAT3 GetRotation() const { return m_rotation; }
	bool IsMoving() const { return m_isMoving; }
	bool IsRunning() const { return m_isRunning; }
	float GetStaminaPercentage() const;

private:
	static constexpr float COLLISION_RADIUS = 0.45f;

	bool IsCollidingWithWall(const DirectX::XMFLOAT3& position, float radius, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);

	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_rotation;

	float m_moveSpeed;
	float m_runSpeed;
	float m_rotationSpeed;
	bool m_isMoving;
	bool m_isRunning;

	float m_stamina;               // 現在のスタミナ
	float m_maxStamina;            // 最大スタミナ
	float m_staminaDepletionRate;  // スタミナ減少率（1秒あたり）
	float m_staminaRegenRate;      // スタミナ回復率（1秒あたり）
	bool m_isStaminaExhausted;     // スタミナが枯渇したかどうかのフラグ
	float m_staminaRechargeThreshold; // 再び走れるようになるために必要なスタミナ量
	float m_staminaWarningThreshhold;
	float m_slowDepletionFactor;
	float m_staminaRegenCoolDown;

	DirectX::SoundEffect* m_walkSound;
	DirectX::SoundEffect* m_runSound;
	float m_stepTimer;
	float m_walkInterval;
	float m_runInterval;
};