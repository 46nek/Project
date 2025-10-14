#include "LightManager.h"
#include "MazeGenerator.h"
#include "Stage.h"
#include <vector>
#include <tuple>

LightManager::LightManager()
{
}

LightManager::~LightManager()
{
}

void LightManager::Initialize(const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth, float wallHeight)
{
    m_lights.clear(); // �����̃��C�g���N���A

    const int maze_width = static_cast<int>(mazeData[0].size());
    const int maze_height = static_cast<int>(mazeData.size());

    // --- �����̒����ɂ̂݃|�C���g���C�g��ݒu ---
    const int roomSize = 3;
    const int cornerOffset = 1;
    using Rect = std::tuple<int, int, int, int>;
    const std::vector<Rect> rooms = {
        // �l���̕���
        {cornerOffset, cornerOffset, roomSize, roomSize},
        {maze_width - cornerOffset - roomSize, cornerOffset, roomSize, roomSize},
        {cornerOffset, maze_height - cornerOffset - roomSize, roomSize, roomSize},
        {maze_width - cornerOffset - roomSize, maze_height - cornerOffset - roomSize, roomSize, roomSize},
        // �����̕���
        {(maze_width - roomSize) / 2, (maze_height - roomSize) / 2, roomSize, roomSize}
    };

    for (const auto& r : rooms) {
        // �����̒��S���W���v�Z
        int roomX = std::get<0>(r) + std::get<2>(r) / 2;
        int roomZ = std::get<1>(r) + std::get<3>(r) / 2;

        Light roomLight = {};
        roomLight.Enabled = true;
        roomLight.Type = PointLight; // �����S�̂��Ƃ炷�|�C���g���C�g
        roomLight.Color = { 1.0f, 0.8f, 0.6f, 1.0f };

        roomLight.Intensity = 2.0f;
        roomLight.Range = 15.0f;  
        roomLight.Attenuation = { 0.1f, 0.4f, 0.2f }; 

        roomLight.Position = { (roomX + 0.5f) * pathWidth, wallHeight - 1.0f, (roomZ + 0.5f) * pathWidth };
        m_lights.push_back(roomLight);
    }

    // --- �ŏI���� ---
    // �V�F�[�_�[�ɓn�����C�g�̐���ݒ�
    m_lightBuffer.NumLights = static_cast<int>(m_lights.size());
    if (m_lightBuffer.NumLights > 16) {
        m_lightBuffer.NumLights = 16;
    }
    for (int i = 0; i < m_lightBuffer.NumLights; ++i) {
        m_lightBuffer.Lights[i] = m_lights[i];
    }

    // �V���h�E�}�b�s���O�p�̐ݒ�
    float mazeWorldWidth = maze_width * pathWidth;
    float mazeWorldHeight = maze_height * pathWidth;
    DirectX::XMVECTOR lightPos = DirectX::XMVectorSet(mazeWorldWidth / 2.0f, 50.0f, mazeWorldHeight / 2.0f, 1.0f);
    DirectX::XMVECTOR lightLookAt = DirectX::XMVectorSet(mazeWorldWidth / 2.0f, 0.0f, mazeWorldHeight / 2.0f, 1.0f);
    DirectX::XMVECTOR lightUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    m_lightViewMatrix = DirectX::XMMatrixLookAtLH(lightPos, lightLookAt, lightUp);
    m_lightProjectionMatrix = DirectX::XMMatrixOrthographicLH(mazeWorldWidth, mazeWorldHeight, 0.1f, 100.0f);
}

void LightManager::Update(float deltaTime, const DirectX::XMFLOAT3& cameraPosition)
{
    m_lightBuffer.CameraPosition = cameraPosition;
}