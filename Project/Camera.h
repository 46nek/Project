// Camera.h
#pragma once
#include <DirectXMath.h>

using namespace DirectX;

class Camera
{
public:
    Camera();
    ~Camera();

    void SetPosition(float x, float y, float z);
    void SetRotation(float x, float y, float z);

    XMFLOAT3 GetPosition();
    XMFLOAT3 GetRotation();

    void Update();
    XMMATRIX GetViewMatrix();

    void MoveForward();
    void MoveBackward();
    void MoveLeft();
    void MoveRight();

private:
    float m_positionX, m_positionY, m_positionZ;
    float m_rotationX, m_rotationY, m_rotationZ;
    XMMATRIX m_viewMatrix;
    
    float m_frameTime;
    float m_moveSpeed;
    float m_rotationSpeed;
};