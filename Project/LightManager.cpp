#include "LightManager.h"
#include "MazeGenerator.h" // MazeGeneratorをインクルード
#include "Stage.h"         // Stageをインクルード
#include <vector>

LightManager::LightManager()
{
}

LightManager::~LightManager()
{
}

void LightManager::Initialize(const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth, float wallHeight)
{
    m_lights.clear(); // 既存のライトをクリア

    // 迷路の通路にライトを配置
    for (int z = 0; z < mazeData.size(); ++z)
    {
        for (int x = 0; x < mazeData[z].size(); ++x)
        {
            // 通路であり、一定間隔で配置する
            if (mazeData[z][x] == MazeGenerator::Path && (x % 5 == 0) && (z % 5== 0))
            {
                Light newLight = {};
                newLight.Enabled = true;
                newLight.Type = SpotLight;
                newLight.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
                newLight.Intensity = 1.0f; // ライトの強度
                newLight.Range = 10.0f;    // 照らす範囲
                newLight.SpotAngle = 0.7f; // スポットライトの角度
                newLight.Attenuation = { 0.1f, 0.2f, 0.1f }; // 距離による減衰

                // 天井から床向きに設定
                newLight.Position = { (x + 0.5f) * pathWidth, wallHeight + 0.5f, (z + 0.5f) * pathWidth };
                newLight.Direction = { 0.0f, -1.0f, 0.0f };

                m_lights.push_back(newLight);
            }
        }
    }

    // シェーダーに渡すライトの数を設定
    m_lightBuffer.NumLights = static_cast<int>(m_lights.size());
    if (m_lightBuffer.NumLights > 16) {
        m_lightBuffer.NumLights = 16; // 上限を超えないように
    }
    for (int i = 0; i < m_lightBuffer.NumLights; ++i) {
        m_lightBuffer.Lights[i] = m_lights[i];
    }

    // シャドウマッピング用の設定（シーン全体を照らす単一のライトを想定）
    DirectX::XMVECTOR lightPos = DirectX::XMVectorSet(15.0f, 20.0f, 15.0f, 1.0f);
    DirectX::XMVECTOR lightLookAt = DirectX::XMVectorSet(15.0f, 0.0f, 15.0f, 1.0f);
    DirectX::XMVECTOR lightUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    m_lightViewMatrix = DirectX::XMMatrixLookAtLH(lightPos, lightLookAt, lightUp);
    m_lightProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, 1.0f, 0.1f, 100.0f);
}

void LightManager::Update(float deltaTime, const DirectX::XMFLOAT3& cameraPosition)
{
    // カメラ位置をシェーダーに渡す
    m_lightBuffer.CameraPosition = cameraPosition;
}