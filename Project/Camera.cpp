#include "Camera.h"
#include <cmath>

Camera::Camera()
{
	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_positionZ = 0.0f;

	m_rotationX = 0.0f;
	m_rotationY = 0.0f;
	m_rotationZ = 0.0f;

	m_basePosition = { 0.0f, 0.0f, 0.0f };

	m_viewMatrix = DirectX::XMMatrixIdentity();
	m_previousViewMatrix = DirectX::XMMatrixIdentity(); // <--- 追加

	m_moveSpeed = 5.0f;
	m_rotationSpeed = 5.0f;

	m_bobbingTimer = 0.0f;
	m_bobbingAmount = 0.03f;
	m_swayAmount = 0.05f;
	m_rollAmount = 0.1f;
	m_bobbingSpeed = 14.0f;
	m_swaySpeed = 7.0f;
	m_rollSpeed = 7.0f;
}

Camera::~Camera()
{
}

void Camera::SetPosition(float x, float y, float z)
{
	m_basePosition = { x, y, z };
}

void Camera::SetRotation(float x, float y, float z)
{
	m_rotationX = x;
	m_rotationY = y;
	m_rotationZ = z;
}

DirectX::XMFLOAT3 Camera::GetPosition() const
{
	return DirectX::XMFLOAT3(m_positionX, m_positionY, m_positionZ);
}

DirectX::XMFLOAT3 Camera::GetRotation() const
{
	return DirectX::XMFLOAT3(m_rotationX, m_rotationY, m_rotationZ);
}

void Camera::Update()
{
	// 現在のビュー行列を前のフレームのものとして保存
	m_previousViewMatrix = m_viewMatrix; // <--- 追加

	DirectX::XMFLOAT3 up, position, lookAt;
	DirectX::XMVECTOR upVector, positionVector, lookAtVector;
	float yaw, pitch, roll;
	DirectX::XMMATRIX rotationMatrix;

	up = { 0.0f, 1.0f, 0.0f };
	upVector = DirectX::XMLoadFloat3(&up);

	position = { m_positionX, m_positionY, m_positionZ };
	positionVector = DirectX::XMLoadFloat3(&position);

	lookAt = { 0.0f, 0.0f, 1.0f };

	pitch = m_rotationX * (DirectX::XM_PI / 180.0f);
	yaw = m_rotationY * (DirectX::XM_PI / 180.0f);
	roll = m_rotationZ * (DirectX::XM_PI / 180.0f);

	rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	lookAtVector = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&lookAt), rotationMatrix);
	upVector = DirectX::XMVector3TransformCoord(upVector, rotationMatrix);

	lookAtVector = DirectX::XMVectorAdd(positionVector, lookAtVector);

	m_viewMatrix = DirectX::XMMatrixLookAtLH(positionVector, lookAtVector, upVector);
}

DirectX::XMMATRIX Camera::GetViewMatrix() const
{
	return m_viewMatrix;
}

// <--- 以下をすべて追加 --->
DirectX::XMMATRIX Camera::GetPreviousViewMatrix() const
{
	return m_previousViewMatrix;
}

void Camera::MoveForward(float deltaTime)
{
	float radians = m_rotationY * 0.0174532925f;
	m_positionX += sinf(radians) * m_moveSpeed * deltaTime;
	m_positionZ += cosf(radians) * m_moveSpeed * deltaTime;
}

void Camera::MoveBackward(float deltaTime)
{
	float radians = m_rotationY * 0.0174532925f;
	m_positionX -= sinf(radians) * m_moveSpeed * deltaTime;
	m_positionZ -= cosf(radians) * m_moveSpeed * deltaTime;
}

void Camera::MoveLeft(float deltaTime)
{
	float radians = (m_rotationY - 90.0f) * 0.0174532925f;
	m_positionX += sinf(radians) * m_moveSpeed * deltaTime;
	m_positionZ += cosf(radians) * m_moveSpeed * deltaTime;
}

void Camera::MoveRight(float deltaTime)
{
	float radians = (m_rotationY + 90.0f) * 0.0174532925f;
	m_positionX += sinf(radians) * m_moveSpeed * deltaTime;
	m_positionZ += cosf(radians) * m_moveSpeed * deltaTime;
}

void Camera::Turn(int mouseX, int mouseY, float deltaTime)
{
	float yaw = (float)mouseX * m_rotationSpeed * deltaTime;
	float pitch = (float)mouseY * m_rotationSpeed * deltaTime;

	m_rotationY += yaw;
	m_rotationX += pitch;

	if (m_rotationX > 90.0f) m_rotationX = 90.0f;
	if (m_rotationX < -90.0f) m_rotationX = -90.0f;
}

void Camera::SetBobbingParameters(float bobbingSpeed, float bobbingAmount, float swaySpeed, float swayAmount, float rollSpeed)
{
	m_bobbingSpeed = bobbingSpeed;
	m_bobbingAmount = bobbingAmount;
	m_swaySpeed = swaySpeed;
	m_swayAmount = swayAmount;
	m_rollSpeed = rollSpeed;
}

void Camera::UpdateBobbing(float deltaTime, bool isMoving)
{
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationY(m_rotationY * (DirectX::XM_PI / 180.0f));

	if (isMoving)
	{
		m_bobbingTimer += deltaTime;
		float bobOffset = sinf(m_bobbingTimer * m_bobbingSpeed) * m_bobbingAmount;
		float swayOffset = sinf(m_bobbingTimer * m_swaySpeed) * m_swayAmount;
		DirectX::XMVECTOR localOffset = DirectX::XMVectorSet(swayOffset, bobOffset, 0.0f, 0.0f);
		DirectX::XMVECTOR worldOffset = DirectX::XMVector3Transform(localOffset, rotationMatrix);
		m_positionX = m_basePosition.x + DirectX::XMVectorGetX(worldOffset);
		m_positionY = m_basePosition.y + DirectX::XMVectorGetY(worldOffset);
		m_positionZ = m_basePosition.z + DirectX::XMVectorGetZ(worldOffset);
	}
	else
	{
		m_bobbingTimer = 0.0f;
		m_positionX += (m_basePosition.x - m_positionX) * 0.1f;
		m_positionY += (m_basePosition.y - m_positionY) * 0.1f;
		m_positionZ += (m_basePosition.z - m_positionZ) * 0.1f;
		m_rotationZ += (0.0f - m_rotationZ) * 0.1f;
	}
}