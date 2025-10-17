// Project/Player.h

#pragma once
#include <DirectXMath.h>
#include <vector>
#include "Input.h"
#include "MazeGenerator.h"
#include <algorithm> // std::max, std::min のために追加

class Player
{
public:
	Player();

	void Initialize(const DirectX::XMFLOAT3& startPosition);
	void Update(float deltaTime, Input* input, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);
	void Turn(int mouseX, int mouseY, float deltaTime);

	DirectX::XMFLOAT3 GetPosition() const { return m_position; }
	DirectX::XMFLOAT3 GetRotation() const { return m_rotation; }
	bool IsMoving() const { return m_isMoving; }
	bool IsRunning() const { return m_isRunning; }

	/**
	 * @brief 現在のスタミナの割合を 0.0f ～ 1.0f の範囲で取得します。
	 * @return float スタミナの割合
	 */
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
};