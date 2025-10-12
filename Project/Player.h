#pragma once
#include <DirectXMath.h>
#include <vector>
#include "Input.h"
#include "MazeGenerator.h"

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

private:
    bool IsCollidingWithWall(const DirectX::XMFLOAT3& position, float radius, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);

    DirectX::XMFLOAT3 m_position;
    DirectX::XMFLOAT3 m_rotation;

    float m_moveSpeed;
    float m_rotationSpeed;
    bool m_isMoving;
};