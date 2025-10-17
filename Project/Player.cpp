// Project/Player.cpp

#include "Player.h"

Player::Player()
	: m_position({ 0.0f, 0.0f, 0.0f }),
	m_rotation({ 0.0f, 0.0f, 0.0f }),
	m_moveSpeed(5.0f),
	m_runSpeed(8.5f),
	m_rotationSpeed(5.0f),
	m_isMoving(false),
	m_isRunning(false),
	m_maxStamina(100.0f),
	m_stamina(100.0f),
	m_staminaDepletionRate(20.0f), // �����Ă����5�b�ŃX�^�~�i���s����v�Z
	m_staminaRegenRate(15.0f),     // �~�܂��Ă���Ɩ�6.7�b�őS�񕜂���v�Z
	m_isStaminaExhausted(false),
	m_staminaRechargeThreshold(25.0f) // �ő�X�^�~�i��25%
{
}

void Player::Initialize(const DirectX::XMFLOAT3& startPosition)
{
	m_position = startPosition;
	m_stamina = m_maxStamina;
}

void Player::Update(float deltaTime, Input* input, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth)
{
	m_isMoving = false;

	bool isTryingToRun = input->IsKeyDown(0x10); // Shift�L�[

	// ������ ��������X�^�~�i�Ƒ���̃��W�b�N��S�ʓI�ɏC�� ������

	// --- �X�^�~�i�̏�ԊǗ� ---
	// �����u�X�^�~�i�؂�v��ԂȂ�A���ʉ񕜂������`�F�b�N����
	if (m_isStaminaExhausted)
	{
		if (m_stamina >= m_staminaRechargeThreshold)
		{
			// 臒l�܂ŉ񕜂����̂ŁA�u�X�^�~�i�؂�v��Ԃ�����
			m_isStaminaExhausted = false;
		}
	}

	// --- ���邩�ǂ����̌��� ---
	// Shift�L�[��������Ă��āA���u�X�^�~�i�؂�v�łȂ��ꍇ�ɑ���
	m_isRunning = isTryingToRun && !m_isStaminaExhausted;

	// --- �X�^�~�i�̍X�V ---
	if (m_isRunning)
	{
		// �����Ă���ꍇ�̓X�^�~�i������
		m_stamina -= m_staminaDepletionRate * deltaTime;

		// �����A���̃t���[���ŃX�^�~�i��0�ɂȂ�����
		if (m_stamina <= 0.0f)
		{
			m_stamina = 0.0f;           // �X�^�~�i��0�ɌŒ�
			m_isRunning = false;        // �����ɑ������߂�
			m_isStaminaExhausted = true; // �u�X�^�~�i�؂�v��Ԃɂ���
		}
	}
	else
	{
		// �����Ă��Ȃ��ꍇ�̓X�^�~�i����
		float currentRegenRate = m_staminaRegenRate;
		if (input->IsKeyDown('W') || input->IsKeyDown('S') || input->IsKeyDown('A') || input->IsKeyDown('D'))
		{
			currentRegenRate *= 0.7f;
		}
		m_stamina += currentRegenRate * deltaTime;

		// �X�^�~�i���ő�l�𒴂��Ȃ��悤�ɂ���
		m_stamina = std::min(m_stamina, m_maxStamina);
	}

	// --- ���x�̌��� ---
	float currentSpeed = m_isRunning ? m_runSpeed : m_moveSpeed;

	// ������ �C�������܂� ������

	float moveAmount = currentSpeed * deltaTime;

	DirectX::XMFLOAT3 desiredMove = { 0, 0, 0 };
	float yaw = m_rotation.y * (DirectX::XM_PI / 180.0f);

	if (input->IsKeyDown('W')) { desiredMove.z += cosf(yaw); desiredMove.x += sinf(yaw); }
	if (input->IsKeyDown('S')) { desiredMove.z -= cosf(yaw); desiredMove.x -= sinf(yaw); }
	if (input->IsKeyDown('A')) { desiredMove.z += sinf(yaw); desiredMove.x -= cosf(yaw); }
	if (input->IsKeyDown('D')) { desiredMove.z -= sinf(yaw); desiredMove.x += cosf(yaw); }

	if (desiredMove.x != 0.0f || desiredMove.z != 0.0f)
	{
		m_isMoving = true;
		DirectX::XMVECTOR moveVec = DirectX::XMLoadFloat3(&desiredMove);
		moveVec = DirectX::XMVector3Normalize(moveVec);
		DirectX::XMStoreFloat3(&desiredMove, DirectX::XMVectorScale(moveVec, moveAmount));

		DirectX::XMFLOAT3 nextPosition = m_position;
		nextPosition.x += desiredMove.x;
		if (!IsCollidingWithWall(nextPosition, COLLISION_RADIUS, mazeData, pathWidth))
		{
			m_position.x = nextPosition.x;
		}

		nextPosition = m_position;
		nextPosition.z += desiredMove.z;
		if (!IsCollidingWithWall(nextPosition, COLLISION_RADIUS, mazeData, pathWidth))
		{
			m_position.z = nextPosition.z;
		}
	}
}

void Player::Turn(int mouseX, int mouseY, float deltaTime)
{
	m_rotation.y += (float)mouseX * m_rotationSpeed * deltaTime;
	m_rotation.x += (float)mouseY * m_rotationSpeed * deltaTime;

	if (m_rotation.x > 90.0f) m_rotation.x = 90.0f;
	if (m_rotation.x < -90.0f) m_rotation.x = -90.0f;
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