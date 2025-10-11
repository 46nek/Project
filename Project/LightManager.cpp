#include "LightManager.h"
#include <cstdlib> // rand() のために追加
#include <ctime>   // time() のために追加

LightManager::LightManager() : m_flickerTimer(0.0f), m_originalIntensity(0.0f)
{
    // 乱数のシードを初期化
    srand(static_cast<unsigned int>(time(NULL)));
}

LightManager::~LightManager() {}

void LightManager::Initialize()
{
    Light playerLight = {};
    playerLight.Enabled = true;
    playerLight.Type = SpotLight;
    playerLight.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
    playerLight.Intensity = 1.8f;
    playerLight.Range = 20.0f;
    playerLight.SpotAngle = 0.95f;
    playerLight.Attenuation = { 0.2f, 0.4f, 0.1f };
    m_lights.push_back(playerLight);

    m_originalIntensity = playerLight.Intensity;

    DirectX::XMVECTOR lightPos = DirectX::XMVectorSet(15.0f, 20.0f, 15.0f, 1.0f);
    DirectX::XMVECTOR lightLookAt = DirectX::XMVectorSet(15.0f, 0.0f, 15.0f, 1.0f);
    DirectX::XMVECTOR lightUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    m_lightViewMatrix = DirectX::XMMatrixLookAtLH(lightPos, lightLookAt, lightUp);
    m_lightProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, 1.0f, 0.1f, 100.0f);
}

void LightManager::Update(float deltaTime, const DirectX::XMFLOAT3& playerPosition, const DirectX::XMFLOAT3& cameraRotation)
{
    // プレイヤーライトの位置と向きの更新
    m_lights[0].Position = playerPosition;

    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(
        cameraRotation.x * (DirectX::XM_PI / 180.0f),
        cameraRotation.y * (DirectX::XM_PI / 180.0f),
        cameraRotation.z * (DirectX::XM_PI / 180.0f)
    );
    DirectX::XMVECTOR lookAt = DirectX::XMVector3TransformCoord({ 0, 0, 1 }, rotationMatrix);
    DirectX::XMStoreFloat3(&m_lights[0].Direction, lookAt);

    m_flickerTimer += deltaTime;
    // 0.05秒ごとにちらつくかどうかを判定
    if (m_flickerTimer > 0.05f)
    {
        // 普段は元の明るさに戻す
        m_lights[0].Intensity = m_originalIntensity;

        // 5%の確率でちらつかせる
        if ((rand() % 100) < 5)
        {
            // 明るさをランダムに元の0%～50%の間の値にする
            float randomDim = static_cast<float>(rand() % 50) / 100.0f;
            m_lights[0].Intensity = m_originalIntensity * randomDim;
        }

        m_flickerTimer = 0.0f; // タイマーをリセット
    }

    // シェーダーに渡すバッファを更新
    m_lightBuffer.NumLights = static_cast<int>(m_lights.size());
    m_lightBuffer.CameraPosition = playerPosition;
    for (int i = 0; i < m_lightBuffer.NumLights; ++i) {
        m_lightBuffer.Lights[i] = m_lights[i];
    }
}