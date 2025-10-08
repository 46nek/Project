// Camera.cpp
#include "Camera.h"

Camera::Camera()
{
    m_positionX = 0.0f;
    m_positionY = 0.0f;
    m_positionZ = 0.0f;

    m_rotationX = 0.0f;
    m_rotationY = 0.0f;
    m_rotationZ = 0.0f;

    m_viewMatrix = XMMatrixIdentity();

    m_moveSpeed = 5.0f;
    m_rotationSpeed = 5.0f;
}

Camera::~Camera()
{
}

void Camera::SetPosition(float x, float y, float z)
{
    m_positionX = x;
    m_positionY = y;
    m_positionZ = z;
}

void Camera::SetRotation(float x, float y, float z)
{
    m_rotationX = x;
    m_rotationY = y;
    m_rotationZ = z;
}

XMFLOAT3 Camera::GetPosition()
{
    return XMFLOAT3(m_positionX, m_positionY, m_positionZ);
}

XMFLOAT3 Camera::GetRotation()
{
    return XMFLOAT3(m_rotationX, m_rotationY, m_rotationZ);
}

void Camera::Update()
{
    XMFLOAT3 up, position, lookAt;
    XMVECTOR upVector, positionVector, lookAtVector;
    float yaw, pitch, roll;
    XMMATRIX rotationMatrix;

    // 上方向ベクトル
    up.x = 0.0f;
    up.y = 1.0f;
    up.z = 0.0f;
    upVector = XMLoadFloat3(&up);

    // カメラの位置
    position.x = m_positionX;
    position.y = m_positionY;
    position.z = m_positionZ;
    positionVector = XMLoadFloat3(&position);

    // カメラの注視点（最初はZ軸の正方向）
    lookAt.x = 0.0f;
    lookAt.y = 0.0f;
    lookAt.z = 1.0f;

    // 回転をラジアンに変換
    pitch = m_rotationX * (XM_PI / 180.0f);
    yaw = m_rotationY * (XM_PI / 180.0f);
    roll = m_rotationZ * (XM_PI / 180.0f);

    // ヨー、ピッチ、ロールから回転行列を作成
    rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

    // 注視点と上方向ベクトルを回転
    lookAtVector = XMVector3TransformCoord(XMLoadFloat3(&lookAt), rotationMatrix);
    upVector = XMVector3TransformCoord(upVector, rotationMatrix);

    // 注視点の最終的な位置を計算
    lookAtVector = XMVectorAdd(positionVector, lookAtVector);

    // ビュー行列を作成
    m_viewMatrix = XMMatrixLookAtLH(positionVector, lookAtVector, upVector);
}

XMMATRIX Camera::GetViewMatrix()
{
    return m_viewMatrix;
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

    // ヨー（左右の回転）を更新
    m_rotationY += yaw;

    // ピッチ（上下の回転）を更新
    m_rotationX += pitch;

    // ピッチが90度以上または-90度以下にならないように制限（カメラがひっくり返るのを防ぐ）
    if (m_rotationX > 90.0f)
    {
        m_rotationX = 90.0f;
    }
    if (m_rotationX < -90.0f)
    {
        m_rotationX = -90.0f;
    }
}