// Camera.h (Š®‘Sã‘‚«)
#pragma once
#include <DirectXMath.h>

class Camera
{
public:
	Camera();
	~Camera();

	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	DirectX::XMFLOAT3 GetPosition() const;
	DirectX::XMFLOAT3 GetRotation() const;

	void Update();

	DirectX::XMMATRIX GetViewMatrix() const;
	DirectX::XMMATRIX GetPreviousViewMatrix() const; // <--- ’Ç‰Á

	void MoveForward(float deltaTime);
	void MoveBackward(float deltaTime);
	void MoveLeft(float deltaTime);
	void MoveRight(float deltaTime);
	void Turn(int mouseX, int mouseY, float deltaTime);
	void UpdateBobbing(float deltaTime, bool isMoving);
	void SetBobbingParameters(float bobbingSpeed, float bobbingAmount, float swaySpeed, float swayAmount, float rollSpeed);

private:
	float m_positionX, m_positionY, m_positionZ;
	DirectX::XMFLOAT3 m_basePosition;
	float m_rotationX, m_rotationY, m_rotationZ;

	DirectX::XMMATRIX m_viewMatrix;
	DirectX::XMMATRIX m_previousViewMatrix;

	float m_moveSpeed;
	float m_rotationSpeed;

	float m_bobbingTimer;
	float m_bobbingAmount;
	float m_bobbingSpeed;
	float m_swayAmount;
	float m_swaySpeed;
	float m_rollAmount;
	float m_rollSpeed;
};