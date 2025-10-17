// Project/Player.h

#pragma once
#include <DirectXMath.h>
#include <vector>
#include "Input.h"
#include "MazeGenerator.h"
#include <algorithm> // std::max, std::min �̂��߂ɒǉ�

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
	 * @brief ���݂̃X�^�~�i�̊����� 0.0f �` 1.0f �͈̔͂Ŏ擾���܂��B
	 * @return float �X�^�~�i�̊���
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

	float m_stamina;               // ���݂̃X�^�~�i
	float m_maxStamina;            // �ő�X�^�~�i
	float m_staminaDepletionRate;  // �X�^�~�i�������i1�b������j
	float m_staminaRegenRate;      // �X�^�~�i�񕜗��i1�b������j
	bool m_isStaminaExhausted;     // �X�^�~�i���͊��������ǂ����̃t���O
	float m_staminaRechargeThreshold; // �Ăё����悤�ɂȂ邽�߂ɕK�v�ȃX�^�~�i��
};