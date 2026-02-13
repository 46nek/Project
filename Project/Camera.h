#pragma once
#include <DirectXMath.h>

/**
 * @brief 繧ｫ繝｡繝ｩ繧ｯ繝ｩ繧ｹ
 */
class Camera {
public:
	Camera(float x = 0.0f, float y = 0.0f, float z = 0.0f);
	~Camera();

	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	DirectX::XMFLOAT3 GetPosition() const;
	DirectX::XMFLOAT3 GetRotation() const;

	void Update(float deltaTime);

	void SetTargetFOV(float fov); // 逶ｮ讓僥OV繧定ｨｭ螳・
	float GetFOV() const;         // 迴ｾ蝨ｨ縺ｮFOV繧貞叙蠕・

	DirectX::XMMATRIX GetViewMatrix() const;
	DirectX::XMMATRIX GetPreviousViewMatrix() const;

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

	float m_fov;            // 迴ｾ蝨ｨ縺ｮFOV
	float m_targetFov;      // 逶ｮ讓吶・FOV

	float m_bobbingTimer;
	float m_bobbingAmount;
	float m_bobbingSpeed;
	float m_swayAmount;
	float m_swaySpeed;
	float m_rollAmount;
	float m_rollSpeed;
};
