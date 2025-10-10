#pragma once
#include <DirectXMath.h>

class Camera
{
public:
    Camera();
    ~Camera();

    void SetPosition(float x, float y, float z);
    void SetRotation(float x, float y, float z);

    DirectX::XMFLOAT3 GetPosition() const; // const ‚ð’Ç‰Á
    DirectX::XMFLOAT3 GetRotation() const; // const ‚ð’Ç‰Á

    void Update();
    DirectX::XMMATRIX GetViewMatrix() const; // const ‚ð’Ç‰Á

    void MoveForward(float deltaTime);
    void MoveBackward(float deltaTime);
    void MoveLeft(float deltaTime);
    void MoveRight(float deltaTime);
    void Turn(int mouseX, int mouseY, float deltaTime);

private:
    float m_positionX, m_positionY, m_positionZ;
    float m_rotationX, m_rotationY, m_rotationZ;
    DirectX::XMMATRIX m_viewMatrix;
    float m_moveSpeed;
    float m_rotationSpeed;
};