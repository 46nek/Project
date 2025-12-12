#pragma once
#include <DirectXMath.h>
#include <vector>
#include "Input.h"
#include "MazeGenerator.h"
#include <algorithm>
#include <memory>

// 前方宣言
namespace DirectX {
	class SoundEffect;
	class SoundEffectInstance;
}

class Player {
public:
	Player();
	~Player();

	void Initialize(const DirectX::XMFLOAT3& startPosition);
	void Update(float deltaTime, Input* input, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);

	// 名前を UpdateSkill に変更
	void UpdateSkill(float deltaTime, Input* input);

	void UpdateMovement(float deltaTime, Input* input, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);
	void UpdateAudio(float deltaTime);
	void Turn(int mouseX, int mouseY, float deltaTime);

	void SetRotation(const DirectX::XMFLOAT3& rotation);
	void SetFootstepSounds(DirectX::SoundEffect* walkSound, DirectX::SoundEffect* runSound);

	DirectX::XMFLOAT3 GetPosition() const { return m_position; }
	DirectX::XMFLOAT3 GetRotation() const { return m_rotation; }
	bool IsMoving() const { return m_isMoving; }

	// スキル状態の取得用メソッド
	bool IsSkillActive() const { return m_isRunning; }
	float GetSkillDurationTimer() const { return m_skillDurationTimer; }
	float GetSkillCooldownTimer() const { return m_skillCoolDownTimer; }

private:
	static constexpr float COLLISION_RADIUS = 0.45f;

	// スキル定数
	static constexpr float RUN_SKILL_DURATION = 3.0f;  // 3秒間走れる
	static constexpr float RUN_SKILL_COOLDOWN = 10.0f; // 再使用まで10秒

	bool IsCollidingWithWall(const DirectX::XMFLOAT3& position, float radius, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);

	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_rotation;

	float m_moveSpeed;
	float m_runSpeed;
	float m_rotationSpeed;
	bool m_isMoving;
	bool m_isRunning;

	// スキル用タイマー
	float m_skillDurationTimer; // 走れる残り時間
	float m_skillCoolDownTimer; // 再使用までの時間

	DirectX::SoundEffect* m_walkSound;
	DirectX::SoundEffect* m_runSound;
	std::unique_ptr<DirectX::SoundEffectInstance> m_walkInstance;
	std::unique_ptr<DirectX::SoundEffectInstance> m_runInstance;

	float m_stepTimer;
	float m_walkInterval;
	float m_runInterval;
};