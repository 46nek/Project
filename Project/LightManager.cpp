#include "LightManager.h"

LightManager::LightManager() : m_flickerTimer(0.0f) {}
LightManager::~LightManager() {}

void LightManager::Initialize()
{
    // �v���C���[�Ǐ]���C�g
    Light playerLight = {};
    playerLight.Enabled = true;
    playerLight.Type = PointLight;
    playerLight.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
    playerLight.Intensity = 1.0f;
    playerLight.Range = 10.0f;
    playerLight.Attenuation = { 0.5f, 0.2f, 0.0f };
    m_lights.push_back(playerLight);

    // ���񃉃C�g
    Light patrolLight = {};
    patrolLight.Enabled = true;
    patrolLight.Type = PointLight;
    patrolLight.Color = { 1.0f, 0.2f, 0.2f, 1.0f };
    patrolLight.Intensity = 1.5f;
    patrolLight.Range = 15.0f;
    patrolLight.Attenuation = { 0.5f, 0.1f, 0.0f };
    m_lights.push_back(patrolLight);

    // �_�ŃX�|�b�g���C�g
    Light spotLight = {};
    spotLight.Enabled = true;
    spotLight.Type = SpotLight;
    spotLight.Color = { 1.0f, 1.0f, 0.5f, 1.0f };
    spotLight.Position = { 10.0f, 5.0f, 10.0f };
    spotLight.Direction = { 0.0f, -1.0f, 0.0f };
    spotLight.Intensity = 2.5f;
    spotLight.Range = 20.0f;
    spotLight.SpotAngle = 0.95f;
    spotLight.Attenuation = { 0.2f, 0.2f, 0.0f };
    m_lights.push_back(spotLight);

    // �V���h�E�}�b�s���O�p�̃��C�g�r���[�E�v���W�F�N�V�����s��
    DirectX::XMVECTOR lightPos = DirectX::XMLoadFloat3(&m_lights[1].Position);
    DirectX::XMVECTOR lightLookAt = DirectX::XMVectorSet(15.0f, 0.0f, 15.0f, 1.0f);
    DirectX::XMVECTOR lightUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    m_lightViewMatrix = DirectX::XMMatrixLookAtLH(lightPos, lightLookAt, lightUp);
    m_lightProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, 1.0f, 0.1f, 100.0f);
}

void LightManager::Update(float deltaTime, const DirectX::XMFLOAT3& playerPosition)
{
    static float totalTime = 0;
    totalTime += deltaTime;

    // ���C�g�̈ʒu���Ԃ��X�V
    m_lights[0].Position = playerPosition;
    m_lights[1].Position.x = sin(totalTime) * 15.0f + 15.0f;
    m_lights[1].Position.z = cos(totalTime) * 15.0f + 15.0f;

    m_flickerTimer += deltaTime;
    if (m_flickerTimer > 0.1f) {
        m_flickerTimer = 0.0f;
        if ((rand() % 100) < 30) {
            m_lights[2].Enabled = !m_lights[2].Enabled;
        }
    }

    // �V�F�[�_�[�ɓn���o�b�t�@���X�V
    m_lightBuffer.NumLights = static_cast<int>(m_lights.size());
    m_lightBuffer.CameraPosition = playerPosition; // �J�����ʒu���X�V
    for (int i = 0; i < m_lightBuffer.NumLights; ++i) {
        m_lightBuffer.Lights[i] = m_lights[i];
    }
}