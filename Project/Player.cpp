#include "Player.h"
#include <Audio.h>
#include <cstdlib>
#include <ctime>  

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
	m_staminaDepletionRate(20.0f), // 走ると5秒で枯渇
	m_staminaRegenRate(15.0f),     // 停止時は約6.7秒で全回復
	m_isStaminaExhausted(false),
	m_staminaRechargeThreshold(30.0f),
	m_staminaWarningThreshhold(30.0f),
	m_slowDepletionFactor(0.6f),
	m_staminaRegenCoolDown(0.0f),
	m_walkSound(nullptr),
	m_runSound(nullptr),
	m_stepTimer(0.0f),
	m_walkInterval(0.5f),
	m_runInterval(0.3f)
{
}

void Player::Initialize(const DirectX::XMFLOAT3& startPosition)
{
	// 乱数シードの初期化
	srand(static_cast<unsigned int>(time(nullptr)));

	m_position = startPosition;
	m_stamina = m_maxStamina;
}

void Player::Update(float deltaTime, Input* input, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth)
{
	m_isMoving = false;

	bool isTryingToRun = input->IsKeyDown(0x10); // Shiftキー

	// --- 走るかどうかの決定 ---
	// Shiftキーが押されていて、かつ「スタミナ切れ」でない場合に走る
	m_isRunning = isTryingToRun && !m_isStaminaExhausted;

	if (m_staminaRegenCoolDown > 0.0f)
	{
		m_staminaRegenCoolDown -= deltaTime;
	}

	// --- スタミナの更新 ---
	if (m_isRunning)
	{
		m_staminaRegenCoolDown = 0.5f;
		float currentDepletionRate = m_staminaDepletionRate;

		// スタミナが少なくなると減少を緩やかにする
		if (m_stamina <= m_staminaWarningThreshhold)
		{
			currentDepletionRate *= m_slowDepletionFactor;
		}
		m_stamina -= currentDepletionRate * deltaTime;

		// スタミナ切れ判定
		if (m_stamina <= 0.0f)
		{
			m_stamina = 0.0f;
			m_isRunning = false;
			m_isStaminaExhausted = true;
			m_staminaRegenCoolDown = 1.0f; // 回復開始までのクールダウン
		}
	}
	else
	{
		// スタミナ回復処理
		if (m_staminaRegenCoolDown <= 0.0f)
		{
			bool isWalking = input->IsKeyDown('W') || input->IsKeyDown('S') || input->IsKeyDown('A') || input->IsKeyDown('D');

			// 移動中またはスタミナ切れ状態なら回復を遅くする
			if (m_isStaminaExhausted || isWalking)
			{
				m_stamina += m_staminaRegenRate * 0.5f * deltaTime;
			}
			else
			{
				m_stamina += m_staminaRegenRate * deltaTime;
			}
			m_stamina = (std::min)(m_stamina, m_maxStamina);

			// スタミナ切れからの復帰判定
			if (m_isStaminaExhausted && m_stamina >= m_staminaRechargeThreshold)
			{
				m_isStaminaExhausted = false;
			}
		}
	}

	// --- 移動速度の決定 ---
	float currentSpeed;
	if (m_isStaminaExhausted)
	{
		currentSpeed = m_moveSpeed * 0.6f;
	}
	else if (m_isRunning)
	{
		currentSpeed = m_runSpeed;
	}
	else
	{
		currentSpeed = m_moveSpeed;
	}

	// --- 移動処理 ---
	float moveAmount = currentSpeed * deltaTime;
	DirectX::XMFLOAT3 desiredMove = { 0, 0, 0 };
	float yaw = m_rotation.y * (DirectX::XM_PI / 180.0f);

	if (input->IsKeyDown('W')) { desiredMove.z += cosf(yaw); desiredMove.x += sinf(yaw); }
	if (input->IsKeyDown('S')) { desiredMove.z -= cosf(yaw); desiredMove.x -= sinf(yaw); }
	if (input->IsKeyDown('A')) { desiredMove.z += sinf(yaw); desiredMove.x -= cosf(yaw); }
	if (input->IsKeyDown('D')) { desiredMove.z -= sinf(yaw); desiredMove.x += cosf(yaw); }

	if (desiredMove.x != 0.0f || desiredMove.z != 0.0f)
	{
		DirectX::XMVECTOR moveVec = DirectX::XMLoadFloat3(&desiredMove);
		moveVec = DirectX::XMVector3Normalize(moveVec);
		DirectX::XMStoreFloat3(&desiredMove, DirectX::XMVectorScale(moveVec, moveAmount));

		// X軸方向の移動と衝突判定
		DirectX::XMFLOAT3 nextPosition = m_position;
		nextPosition.x += desiredMove.x;
		if (!IsCollidingWithWall(nextPosition, COLLISION_RADIUS, mazeData, pathWidth))
		{
			m_position.x = nextPosition.x;
			m_isMoving = true;
		}

		// Z軸方向の移動と衝突判定
		nextPosition = m_position;
		nextPosition.z += desiredMove.z;
		if (!IsCollidingWithWall(nextPosition, COLLISION_RADIUS, mazeData, pathWidth))
		{
			m_position.z = nextPosition.z;
			m_isMoving = true;
		}
	}

	// --- 足音の再生処理 ---
	if (m_isMoving)
	{
		m_stepTimer -= deltaTime;

		if (m_stepTimer <= 0.0f)
		{
			// ランダムなピッチと音量で再生して自然さを出す
			float randomPitch = ((float)rand() / RAND_MAX) * 1.0f - 0.5f;
			float volume = 0.3f;

			if (m_isRunning && !m_isStaminaExhausted)
			{
				// 走る音
				if (m_runSound)
				{
					m_runSound->Play(volume, randomPitch, 0.0f);
				}
				m_stepTimer = m_runInterval;
			}
			else
			{
				// 歩く音
				if (m_walkSound)
				{
					m_walkSound->Play(volume, randomPitch, 0.0f);
				}

				// スタミナ切れ時は足音の間隔を広げる
				if (m_isStaminaExhausted)
				{
					m_stepTimer = 0.7f;
				}
				else
				{
					m_stepTimer = m_walkInterval;
				}
			}
		}
	}
	else
	{
		// 停止中はタイマーを少し溜めた状態にする
		// これにより、動き出した直後ではなく一歩踏み込んだタイミングで音が鳴る
		m_stepTimer = 0.3f;
	}
}

void Player::Turn(int mouseX, int mouseY, float deltaTime)
{
	m_rotation.y += (float)mouseX * m_rotationSpeed;
	m_rotation.x += (float)mouseY * m_rotationSpeed;

	// 上下の視点制限 (-90度 ～ 90度)
	if (m_rotation.x > 90.0f) m_rotation.x = 90.0f;
	if (m_rotation.x < -90.0f) m_rotation.x = -90.0f;
}

void Player::SetFootstepSounds(DirectX::SoundEffect* walkSound, DirectX::SoundEffect* runSound)
{
	m_walkSound = walkSound;
	m_runSound = runSound;
}

bool Player::IsCollidingWithWall(const DirectX::XMFLOAT3& position, float radius, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth)
{
	int mazeHeight = static_cast<int>(mazeData.size());
	int mazeWidth = static_cast<int>(mazeData[0].size());

	int minGridX = static_cast<int>((position.x - radius) / pathWidth);
	int maxGridX = static_cast<int>((position.x + radius) / pathWidth);
	int minGridZ = static_cast<int>((position.z - radius) / pathWidth);
	int maxGridZ = static_cast<int>((position.z + radius) / pathWidth);

	for (int z = minGridZ; z <= maxGridZ; ++z)
	{
		for (int x = minGridX; x <= maxGridX; ++x)
		{
			if (z < 0 || z >= mazeHeight || x < 0 || x >= mazeWidth) return true;
			if (mazeData[z][x] == MazeGenerator::Wall) return true;
		}
	}
	return false;
}

float Player::GetStaminaPercentage() const
{
	if (m_maxStamina <= 0.0f)
	{
		return 0.0f;
	}
	return m_stamina / m_maxStamina;
}