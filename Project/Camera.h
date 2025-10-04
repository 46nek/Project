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

    void MoveForward(float deltaTime);
    void MoveBackward(float deltaTime);
    void MoveLeft(float deltaTime);
    void MoveRight(float deltaTime);

    void Turn(int mouseX, int mouseY, float deltaTime);

private:
    float m_positionX, m_positionY, m_positionZ;
    float m_rotationX, m_rotationY, m_rotationZ;
    XMMATRIX m_viewMatrix;
    
    float m_moveSpeed;
    float m_rotationSpeed;
};