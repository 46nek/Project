#pragma once
#include <DirectXMath.h>
#include <vector>
#include "Input.h"
#include "MazeGenerator.h"
#include <algorithm>
#include <memory> // unique_ptr用

// 前方宣言
namespace DirectX {
	class SoundEffect;
	class SoundEffectInstance;
}

class Player {
public:
	Player();
	~Player(); // デストラクタを明示的に定義

	void Initialize(const DirectX::XMFLOAT3& startPosition);
	void Update(float deltaTime, Input* input, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);
	void UpdateStamina(float deltaTime, Input* input);
	void UpdateMovement(float deltaTime, Input* input, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);
	void UpdateAudio(float deltaTime);
	void Turn(int mouseX, int mouseY, float deltaTime);

	void SetRotation(const DirectX::XMFLOAT3& rotation);
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

	float m_stamina;
	float m_maxStamina;
	float m_staminaDepletionRate;
	float m_staminaRegenRate;
	bool m_isStaminaExhausted;
	float m_staminaRechargeThreshold;
	float m_staminaWarningThreshhold;
	float m_slowDepletionFactor;
	float m_staminaRegenCoolDown;

	DirectX::SoundEffect* m_walkSound; // 参照用（データ元）
	DirectX::SoundEffect* m_runSound;  // 参照用（データ元）
	std::unique_ptr<DirectX::SoundEffectInstance> m_walkInstance; // 再生管理用
	std::unique_ptr<DirectX::SoundEffectInstance> m_runInstance;  // 再生管理用

	float m_stepTimer;
	float m_walkInterval;
	float m_runInterval;
};