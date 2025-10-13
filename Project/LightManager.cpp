#include "LightManager.h"
#include "MazeGenerator.h" // MazeGenerator���C���N���[�h
#include "Stage.h"         // Stage���C���N���[�h
#include <vector>

LightManager::LightManager()
{
}

LightManager::~LightManager()
{
}

void LightManager::Initialize(const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth, float wallHeight)
{
    m_lights.clear(); // �����̃��C�g���N���A

    // ���H�̒ʘH�Ƀ��C�g��z�u
    for (int z = 0; z < mazeData.size(); ++z)
    {
        for (int x = 0; x < mazeData[z].size(); ++x)
        {
            // �ʘH�ł���A���Ԋu�Ŕz�u����
            if (mazeData[z][x] == MazeGenerator::Path && (x % 5 == 0) && (z % 5== 0))
            {
                Light newLight = {};
                newLight.Enabled = true;
                newLight.Type = SpotLight;
                newLight.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
                newLight.Intensity = 1.0f; // ���C�g�̋��x
                newLight.Range = 10.0f;    // �Ƃ炷�͈�
                newLight.SpotAngle = 0.7f; // �X�|�b�g���C�g�̊p�x
                newLight.Attenuation = { 0.1f, 0.2f, 0.1f }; // �����ɂ�錸��

                // �V�䂩�珰�����ɐݒ�
                newLight.Position = { (x + 0.5f) * pathWidth, wallHeight + 0.5f, (z + 0.5f) * pathWidth };
                newLight.Direction = { 0.0f, -1.0f, 0.0f };

                m_lights.push_back(newLight);
            }
        }
    }

    // �V�F�[�_�[�ɓn�����C�g�̐���ݒ�
    m_lightBuffer.NumLights = static_cast<int>(m_lights.size());
    if (m_lightBuffer.NumLights > 16) {
        m_lightBuffer.NumLights = 16; // ����𒴂��Ȃ��悤��
    }
    for (int i = 0; i < m_lightBuffer.NumLights; ++i) {
        m_lightBuffer.Lights[i] = m_lights[i];
    }

    // �V���h�E�}�b�s���O�p�̐ݒ�i�V�[���S�̂��Ƃ炷�P��̃��C�g��z��j
    DirectX::XMVECTOR lightPos = DirectX::XMVectorSet(15.0f, 20.0f, 15.0f, 1.0f);
    DirectX::XMVECTOR lightLookAt = DirectX::XMVectorSet(15.0f, 0.0f, 15.0f, 1.0f);
    DirectX::XMVECTOR lightUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    m_lightViewMatrix = DirectX::XMMatrixLookAtLH(lightPos, lightLookAt, lightUp);
    m_lightProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, 1.0f, 0.1f, 100.0f);
}

void LightManager::Update(float deltaTime, const DirectX::XMFLOAT3& cameraPosition)
{
    // �J�����ʒu���V�F�[�_�[�ɓn��
    m_lightBuffer.CameraPosition = cameraPosition;
}