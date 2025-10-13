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

    m_moveSpeed = 5.0f;
    m_rotationSpeed = 5.0f;

    m_bobbingTimer = 0.0f;
    m_bobbingAmount = 0.03f; // 縦揺れの大きさ
    m_swayAmount = 0.05f;  // 横揺れの大きさ
    m_rollAmount = 0.1f;   // 傾きの大きさ (角度)
    m_bobbingSpeed = 14.0f;  // 縦揺れは速く
    m_swaySpeed = 7.0f;   // 横揺れは縦の半分の速さ
    m_rollSpeed = 7.0f;   // 傾きも横と同じ速さ
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

void Camera::UpdateBobbing(float deltaTime, bool isMoving)
{
    // カメラのヨー（Y軸回転）から回転行列を生成
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationY(m_rotationY * (DirectX::XM_PI / 180.0f));

    if (isMoving)
        {
        m_bobbingTimer += deltaTime;

        // 縦揺れ（ボブ）：速い周期のSin波
        float bobOffset = sinf(m_bobbingTimer * m_bobbingSpeed) * m_bobbingAmount;

        // 横揺れ（スウェイ）：遅い周期のSin波
        float swayOffset = sinf(m_bobbingTimer * m_swaySpeed) * m_swayAmount;

        // 傾き（ロール）：横揺れと同じ周期のCos波（位相をずらすため）
        m_rotationZ = cosf(m_bobbingTimer * m_rollSpeed) * m_rollAmount;

        // 揺れのオフセットをローカル座標として定義
        DirectX::XMVECTOR localOffset = DirectX::XMVectorSet(swayOffset, bobOffset, 0.0f, 0.0f);
        
        // オフセットをカメラの向きに合わせて回転させ、ワールド座標のオフセットに変換
        DirectX::XMVECTOR worldOffset = DirectX::XMVector3Transform(localOffset, rotationMatrix);
        
        // 基準位置にワールド座標のオフセットを加算
        m_positionX = m_basePosition.x + DirectX::XMVectorGetX(worldOffset);
        m_positionY = m_basePosition.y + DirectX::XMVectorGetY(worldOffset);
        m_positionZ = m_basePosition.z + DirectX::XMVectorGetZ(worldOffset);
        }
    else
        {
        // 移動していない場合は、ゆっくりと元の位置と角度に戻す
        m_bobbingTimer = 0.0f;
        m_positionX += (m_basePosition.x - m_positionX) * 0.1f;
        m_positionY += (m_basePosition.y - m_positionY) * 0.1f;
        m_positionZ += (m_basePosition.z - m_positionZ) * 0.1f;
        m_rotationZ += (0.0f - m_rotationZ) * 0.1f;
    }
}