#include "Player.h"

Player::Player()
    : m_position({ 0.0f, 0.0f, 0.0f }),
    m_rotation({ 0.0f, 0.0f, 0.0f }),
    m_moveSpeed(5.0f),
    m_rotationSpeed(5.0f),
    m_isMoving(false)
{
}

void Player::Initialize(const DirectX::XMFLOAT3& startPosition)
{
    m_position = startPosition;
}

void Player::Update(float deltaTime, Input* input, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth)
{
    m_isMoving = false;
    float moveAmount = m_moveSpeed * deltaTime;

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
        if (!IsCollidingWithWall(nextPosition, 0.45f, mazeData, pathWidth))
        {
            m_position.x = nextPosition.x;
        }

        nextPosition = m_position;
        nextPosition.z += desiredMove.z;
        if (!IsCollidingWithWall(nextPosition, 0.45f, mazeData, pathWidth))
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