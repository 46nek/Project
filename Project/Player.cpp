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
	m_runSpeed(8.5f),
	m_rotationSpeed(0.1f),
	m_isMoving(false),
	m_isRunning(false),
	m_maxStamina(100.0f),
	m_stamina(100.0f),
	m_staminaDepletionRate(20.0f),
	m_staminaRegenRate(15.0f),
	m_isStaminaExhausted(false),
	m_staminaRechargeThreshold(30.0f),
	m_staminaWarningThreshhold(30.0f),
	m_slowDepletionFactor(0.6f),
	m_staminaRegenCoolDown(0.0f),
	m_walkSound(nullptr),
	m_runSound(nullptr),
	m_stepTimer(0.0f),
	m_walkInterval(0.5f),
	m_runInterval(0.3f) {
}

Player::~Player() {
	if (m_walkInstance) {
		m_walkInstance->Stop(true); // true = 即時停止
	}
	if (m_runInstance) {
		m_runInstance->Stop(true);
	}
}

void Player::Initialize(const DirectX::XMFLOAT3& startPosition) {
	m_position = startPosition;
	m_stamina = m_maxStamina;
	// rotationはここではリセットせず、SetRotationで制御するか、0初期化のままにする
	// GameScene側でSetRotationを呼ぶ設計にします
}

void Player::Update(float deltaTime, Input* input, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth) {
	m_isMoving = false;

	UpdateStamina(deltaTime, input);
	UpdateMovement(deltaTime, input, mazeData, pathWidth);
	UpdateAudio(deltaTime);
}

void Player::UpdateStamina(float deltaTime, Input* input) {
	bool isTryingToRun = input->IsKeyDown(KEY_SHIFT);
	m_isRunning = isTryingToRun && !m_isStaminaExhausted;

	if (m_staminaRegenCoolDown > 0.0f) {
		m_staminaRegenCoolDown -= deltaTime;
	}

	if (m_isRunning) {
		m_staminaRegenCoolDown = 0.5f;
		float currentDepletionRate = m_staminaDepletionRate;
		if (m_stamina <= m_staminaWarningThreshhold) {
			currentDepletionRate *= m_slowDepletionFactor;
		}
		m_stamina -= currentDepletionRate * deltaTime;

		if (m_stamina <= 0.0f) {
			m_stamina = 0.0f;
			m_isRunning = false;
			m_isStaminaExhausted = true;
			m_staminaRegenCoolDown = 1.0f;
		}
	}
	else {
		if (m_staminaRegenCoolDown <= 0.0f) {
			bool isWalking = input->IsKeyDown(KEY_MOVE_FORWARD) ||
				input->IsKeyDown(KEY_MOVE_BACKWARD) ||
				input->IsKeyDown(KEY_MOVE_LEFT) ||
				input->IsKeyDown(KEY_MOVE_RIGHT);

			if (m_isStaminaExhausted || isWalking) {
				m_stamina += m_staminaRegenRate * 0.5f * deltaTime;
			}
			else {
				m_stamina += m_staminaRegenRate * deltaTime;
			}

			m_stamina = (std::min)(m_stamina, m_maxStamina);
			if (m_isStaminaExhausted && m_stamina >= m_staminaRechargeThreshold) {
				m_isStaminaExhausted = false;
			}
		}
	}
}

void Player::UpdateMovement(float deltaTime, Input* input, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth) {
	float currentSpeed;
	if (m_isStaminaExhausted) {
		currentSpeed = m_moveSpeed * 0.6f;
	}
	else if (m_isRunning) {
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

		// X軸方向の移動と衝突判定
		DirectX::XMFLOAT3 nextPosition = m_position;
		nextPosition.x += desiredMove.x;
		if (!IsCollidingWithWall(nextPosition, COLLISION_RADIUS, mazeData, pathWidth)) {
			m_position.x = nextPosition.x;
			m_isMoving = true;
		}

		// Z軸方向の移動と衝突判定
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

			if (m_isRunning && !m_isStaminaExhausted) {
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

				if (m_isStaminaExhausted) {
					m_stepTimer = 0.7f;
				}
				else {
					m_stepTimer = m_walkInterval;
				}
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

// 追加実装
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

float Player::GetStaminaPercentage() const {
	if (m_maxStamina <= 0.0f) return 0.0f;
	return m_stamina / m_maxStamina;
}