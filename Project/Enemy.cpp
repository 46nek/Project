#include "Enemy.h"
#include "AssetLoader.h"
#include <random>

// �R���X�g���N�^
Enemy::Enemy()
	: m_speed(2.0f),
	m_pathIndex(-1),
	m_pathCooldown(0.0f),
	m_currentState(AIState::Wandering),
	m_stateTimer(0.0f)
{
}

Enemy::~Enemy() {}

// ������
bool Enemy::Initialize(ID3D11Device* device, const DirectX::XMFLOAT3& startPosition, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData)
{
	m_position = startPosition;
	m_model = AssetLoader::LoadModelFromFile(device, "Assets/cube.obj");
	if (!m_model) return false;

	m_model->SetPosition(startPosition.x, startPosition.y, startPosition.z);

	// A*�T���p�̃C���X�^���X���쐬
	m_astar = std::make_unique<AStar>(mazeData);

	return true;
}

void Enemy::Shutdown()
{
	if (m_model) m_model->Shutdown();
}

// �v���C���[�������邩�ǂ����̔���i�ȈՔŁj
bool Enemy::CanSeePlayer(const Player* player, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth)
{
	if (!player) return false;

	DirectX::XMFLOAT3 playerPos = player->GetPosition();
	DirectX::XMVECTOR vecToPlayer = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&playerPos), DirectX::XMLoadFloat3(&m_position));

	// �v���C���[�Ƃ̋������v�Z
	float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(vecToPlayer));

	// ������15.0f��艓����΁u�����Ȃ��v
	if (distance > 15.0f)
	{
		return false;
	}

	// ����͋��������Ŕ���i����̃X�e�b�v�Ŏ��E��ǂ̔����ǉ��j
	return true;
}

// �p�j���[�h�̏���
void Enemy::Wander(float deltaTime, float pathWidth)
{
	m_stateTimer -= deltaTime;
	// �ڕW�n�_�ɓ����������A�^�C�}�[���؂ꂽ��V�����ڕW�n�_��ݒ�
	if (m_pathIndex == -1 || m_stateTimer <= 0.0f)
	{
		m_stateTimer = 5.0f; // 5�b���ƂɐV�����ꏊ��

		int currentX = static_cast<int>(m_position.x / pathWidth);
		int currentY = static_cast<int>(m_position.z / pathWidth);

		int mazeWidth = m_astar->GetMazeWidth();
		int mazeHeight = m_astar->GetMazeHeight();
		int targetX, targetY;

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> distribX(0, mazeWidth - 1);
		std::uniform_int_distribution<> distribY(0, mazeHeight - 1);

		// ���ݒn�����苗���i�}���n�b�^��������5�ȏ�j���ꂽ�L���ȏꏊ��������܂ŒT��������
		do {
			targetX = distribX(gen);
			targetY = distribY(gen);
		} while (abs(currentX - targetX) + abs(currentY - targetY) < 5 || !m_astar->IsWalkable(targetX, targetY));

		// �o�H�T��
		m_path = m_astar->FindPath(currentX, currentY, targetX, targetY);
		m_pathIndex = m_path.empty() ? -1 : 1;
	}
}

// ���t���[���̍X�V����
void Enemy::Update(float deltaTime, const Player* player, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth)
{
	if (!player || !m_model) return;

	// �v���C���[�Ƃ̋������v�Z
	DirectX::XMFLOAT3 playerPos = player->GetPosition();
	float dx = playerPos.x - m_position.x;
	float dz = playerPos.z - m_position.z;
	float distanceSq = (dx * dx) + (dz * dz); // ������2��Ŕ�r

	// === ��ԑJ�ڂ̔��� ===
	switch (m_currentState)
	{
	case AIState::Wandering:
		m_speed = 4.0f;
		if (CanSeePlayer(player, mazeData, pathWidth))
		{
			m_currentState = AIState::Chasing;
			m_pathCooldown = 0.0f;
		}
		break;

	case AIState::Chasing:
		m_speed = 6.0f;
		if (!CanSeePlayer(player, mazeData, pathWidth))
		{
			m_stateTimer += deltaTime;
			if (m_stateTimer > 3.0f)
			{
				m_currentState = AIState::Wandering;
				m_pathIndex = -1;
			}
		}
		else
		{
			m_stateTimer = 0.0f;
		}
		break;
	}

	// === ��Ԃ��Ƃ̍s�� ===
	m_pathCooldown -= deltaTime;
	if (m_pathCooldown <= 0.0f)
	{
		switch (m_currentState)
		{
		case AIState::Wandering:
			// �p�j���[�h�ł�5�b���ƂɐV�����ڕW��ݒ�
			m_pathCooldown = 5.0f;
			Wander(deltaTime, pathWidth); // Wander�֐��͖ڕW�ݒ�݂̂ɗ��p
			break;

		case AIState::Chasing:
			// --- �v���C���[�Ƃ̋����ɉ����Čo�H�T���̕p�x��ύX ---
			if (distanceSq > 15.0f * 15.0f) // ������15�ȏ�Ȃ�
			{
				m_pathCooldown = 3.0f; // 2�b����
			}
			else if (distanceSq > 10.0f * 10.0f) // ������10�ȏ�Ȃ�
			{
				m_pathCooldown = 1.5f; // 1�b����
			}
			else // ������10�����Ȃ�
			{
				m_pathCooldown = 0.8f; // 0.5�b���Ɓi��萳�m�ɒǐՁj
			}

			int startX = static_cast<int>(m_position.x / pathWidth);
			int startY = static_cast<int>(m_position.z / pathWidth);
			int goalX = static_cast<int>(playerPos.x / pathWidth);
			int goalY = static_cast<int>(playerPos.z / pathWidth);
			m_path = m_astar->FindPath(startX, startY, goalX, goalY);
			m_pathIndex = m_path.empty() ? -1 : 1;
			break;
		}
	}

	// === �p�X�ɉ������ړ����� (����) ===
	if (m_pathIndex != -1 && m_pathIndex < m_path.size())
	{
		DirectX::XMFLOAT2 nextGridPos = m_path[m_pathIndex];
		DirectX::XMFLOAT3 targetPosition = {
			(nextGridPos.x + 0.5f) * pathWidth,
			m_position.y,
			(nextGridPos.y + 0.5f) * pathWidth
		};

		DirectX::XMVECTOR vecToTarget = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&targetPosition), DirectX::XMLoadFloat3(&m_position));
		float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(vecToTarget));

		if (distance < 0.1f)
		{
			m_pathIndex++;
		}
		else
		{
			DirectX::XMVECTOR moveVec = DirectX::XMVector3Normalize(vecToTarget);
			moveVec = DirectX::XMVectorScale(moveVec, m_speed * deltaTime);
			DirectX::XMStoreFloat3(&m_position, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&m_position), moveVec));
			m_model->SetPosition(m_position.x, m_position.y, m_position.z);
		}
	}
}

Model* Enemy::GetModel()
{
	return m_model.get();
}

DirectX::XMFLOAT3 Enemy::GetPosition() const
{
	return m_position;
}