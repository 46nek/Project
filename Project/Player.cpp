#include "Player.h"
#include "Audio.h"
#include <cmath>
#include <algorithm>
#include <random> 

namespace {
	// 定数定義
	constexpr int KEY_SHIFT = 0x10;
	constexpr int KEY_MOVE_FORWARD = 'W';
	constexpr int KEY_MOVE_BACKWARD = 'S';
	constexpr int KEY_MOVE_LEFT = 'A';
	constexpr int KEY_MOVE_RIGHT = 'D';
}

Player::Player()
	: m_position({ 0.0f, 0.0f, 0.0f }),
	m_rotation({ 0.0f, 0.0f, 0.0f }),
	m_moveSpeed(5.0f),
	m_runSpeed(12.0f), 
	m_rotationSpeed(0.1f),
	m_isMoving(false),
	m_isRunning(false),
	m_skillDurationTimer(0.0f),
	m_skillCoolDownTimer(0.0f),
	m_walkSound(nullptr),
	m_runSound(nullptr),
	m_stepTimer(0.0f),
	m_walkInterval(0.5f),
	m_runInterval(0.3f) {
}

Player::~Player() {
	if (m_walkInstance) {
		m_walkInstance->Stop(true);
	}
	if (m_runInstance) {
		m_runInstance->Stop(true);
	}
}

void Player::Initialize(const DirectX::XMFLOAT3& startPosition) {
	m_position = startPosition;
	m_skillDurationTimer = 0.0f;
	m_skillCoolDownTimer = 0.0f;
	m_isRunning = false;
}

void Player::Update(float deltaTime, Input* input, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth) {
	m_isMoving = false;

	UpdateSkill(deltaTime, input);
	UpdateMovement(deltaTime, input, mazeData, pathWidth);
	UpdateAudio(deltaTime);
}

void Player::UpdateSkill(float deltaTime, Input* input) {
	// クールダウン経過
	if (m_skillCoolDownTimer > 0.0f) {
		m_skillCoolDownTimer -= deltaTime;
		if (m_skillCoolDownTimer < 0.0f) m_skillCoolDownTimer = 0.0f;
	}

	// 効果時間経過
	if (m_isRunning) {
		m_skillDurationTimer -= deltaTime;
		if (m_skillDurationTimer <= 0.0f) {
			m_isRunning = false;
			m_skillDurationTimer = 0.0f;
		}
	}

	// 発動判定 (クールダウン0 かつ Shiftキーを押した瞬間)
	if (m_skillCoolDownTimer <= 0.0f && input->IsKeyPressed(KEY_SHIFT)) {
		m_isRunning = true;
		m_skillDurationTimer = RUN_SKILL_DURATION;
		m_skillCoolDownTimer = RUN_SKILL_COOLDOWN; // 発動した瞬間にクールダウン開始（10秒）
	}
}

void Player::UpdateMovement(float deltaTime, Input* input, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth) {
	float currentSpeed;

	if (m_isRunning) {
		currentSpeed = m_runSpeed;
	}
	else {
		currentSpeed = m_moveSpeed;
	}

	float moveAmount = currentSpeed * deltaTime;
	DirectX::XMFLOAT3 desiredMove = { 0, 0, 0 };
	float yaw = m_rotation.y * (DirectX::XM_PI / 180.0f);

	if (input->IsKeyDown(KEY_MOVE_FORWARD)) { desiredMove.z += cosf(yaw); desiredMove.x += sinf(yaw); }
	if (input->IsKeyDown(KEY_MOVE_BACKWARD)) { desiredMove.z -= cosf(yaw); desiredMove.x -= sinf(yaw); }
	if (input->IsKeyDown(KEY_MOVE_LEFT)) { desiredMove.z += sinf(yaw); desiredMove.x -= cosf(yaw); }
	if (input->IsKeyDown(KEY_MOVE_RIGHT)) { desiredMove.z -= sinf(yaw); desiredMove.x += cosf(yaw); }

	if (desiredMove.x != 0.0f || desiredMove.z != 0.0f) {
		DirectX::XMVECTOR moveVec = DirectX::XMLoadFloat3(&desiredMove);
		moveVec = DirectX::XMVector3Normalize(moveVec);
		DirectX::XMStoreFloat3(&desiredMove, DirectX::XMVectorScale(moveVec, moveAmount));

		DirectX::XMFLOAT3 nextPosition = m_position;
		nextPosition.x += desiredMove.x;
		if (!IsCollidingWithWall(nextPosition, COLLISION_RADIUS, mazeData, pathWidth)) {
			m_position.x = nextPosition.x;
			m_isMoving = true;
		}

		nextPosition = m_position;
		nextPosition.z += desiredMove.z;
		if (!IsCollidingWithWall(nextPosition, COLLISION_RADIUS, mazeData, pathWidth)) {
			m_position.z = nextPosition.z;
			m_isMoving = true;
		}
	}
}

void Player::UpdateAudio(float deltaTime) {
	if (m_isMoving) {
		m_stepTimer -= deltaTime;

		if (m_stepTimer <= 0.0f) {
			static std::random_device rd;
			static std::mt19937 gen(rd());
			std::uniform_real_distribution<float> dist(-0.5f, 0.5f);

			float randomPitch = dist(gen);
			float volume = 0.3f;

			if (m_isRunning) {
				if (m_runInstance) {
					m_runInstance->Stop();
					m_runInstance->SetVolume(volume);
					m_runInstance->SetPitch(randomPitch);
					m_runInstance->Play();
				}
				m_stepTimer = m_runInterval;
			}
			else {
				if (m_walkInstance) {
					m_walkInstance->Stop();
					m_walkInstance->SetVolume(volume);
					m_walkInstance->SetPitch(randomPitch);
					m_walkInstance->Play();
				}
				m_stepTimer = m_walkInterval;
			}
		}
	}
	else {
		m_stepTimer = 0.3f;
	}
}

void Player::Turn(int mouseX, int mouseY, float deltaTime) {
	m_rotation.y += (float)mouseX * m_rotationSpeed;
	m_rotation.x += (float)mouseY * m_rotationSpeed;
	if (m_rotation.x > 90.0f) m_rotation.x = 90.0f;
	if (m_rotation.x < -90.0f) m_rotation.x = -90.0f;
}

void Player::SetRotation(const DirectX::XMFLOAT3& rotation) {
	m_rotation = rotation;
}

void Player::SetFootstepSounds(DirectX::SoundEffect* walkSound, DirectX::SoundEffect* runSound) {
	m_walkSound = walkSound;
	m_runSound = runSound;

	if (m_walkSound) {
		m_walkInstance = m_walkSound->CreateInstance();
	}
	if (m_runSound) {
		m_runInstance = m_runSound->CreateInstance();
	}
}

bool Player::IsCollidingWithWall(const DirectX::XMFLOAT3& position, float radius, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth) {
	int mazeHeight = static_cast<int>(mazeData.size());
	int mazeWidth = static_cast<int>(mazeData[0].size());

	int minGridX = static_cast<int>((position.x - radius) / pathWidth);
	int maxGridX = static_cast<int>((position.x + radius) / pathWidth);
	int minGridZ = static_cast<int>((position.z - radius) / pathWidth);
	int maxGridZ = static_cast<int>((position.z + radius) / pathWidth);

	for (int z = minGridZ; z <= maxGridZ; ++z) {
		for (int x = minGridX; x <= maxGridX; ++x) {
			if (z < 0 || z >= mazeHeight || x < 0 || x >= mazeWidth) return true;
			if (mazeData[z][x] == MazeGenerator::Wall) return true;
		}
	}
	return false;
}