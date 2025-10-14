#include "LightManager.h"
#include "MazeGenerator.h"
#include "Stage.h"
#include <vector>
#include <tuple>

LightManager::LightManager()
    : m_flashlightIndex(-1),
    m_flashlightBaseIntensity(0.0f), // <--- 追加
    m_flickerTimer(0.0f),
    m_nextFlickerTime(0.0f),
    m_isFlickering(false),
    m_rng(std::random_device{}())
{
}

LightManager::~LightManager()
{
}

void LightManager::Initialize(const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth, float wallHeight)
{
    m_lights.clear();

    const int maze_width = static_cast<int>(mazeData[0].size());
    const int maze_height = static_cast<int>(mazeData.size());

    // --- 部屋のポイントライト ---
    const int roomSize = 3;
    const int cornerOffset = 1;
    using Rect = std::tuple<int, int, int, int>;
    const std::vector<Rect> rooms = {
        {cornerOffset, cornerOffset, roomSize, roomSize},
        {maze_width - cornerOffset - roomSize, cornerOffset, roomSize, roomSize},
        {cornerOffset, maze_height - cornerOffset - roomSize, roomSize, roomSize},
        {maze_width - cornerOffset - roomSize, maze_height - cornerOffset - roomSize, roomSize, roomSize},
        {(maze_width - roomSize) / 2, (maze_height - roomSize) / 2, roomSize, roomSize}
    };

    for (const auto& r : rooms) {
        int roomX = std::get<0>(r) + std::get<2>(r) / 2;
        int roomZ = std::get<1>(r) + std::get<3>(r) / 2;
        Light roomLight = {};
        roomLight.Enabled = true;
        roomLight.Type = PointLight;
        roomLight.Color = { 1.0f, 0.8f, 0.6f, 1.0f };
        roomLight.Intensity = 2.0f;
        roomLight.Range = 15.0f;
        roomLight.Attenuation = { 0.1f, 0.4f, 0.2f };
        roomLight.Position = { (roomX + 0.5f) * pathWidth, wallHeight - 1.0f, (roomZ + 0.5f) * pathWidth };
        m_lights.push_back(roomLight);
    }

    // --- 懐中電灯（スポットライト）を追加 ---
    Light flashlight = {};
    flashlight.Enabled = true;
    flashlight.Type = SpotLight;
    flashlight.Color = { 1.0f, 1.0f, 0.9f, 1.0f };
    m_flashlightBaseIntensity = 0.2f; 
    flashlight.Intensity = m_flashlightBaseIntensity;
    flashlight.Range = 40.0f;       
    flashlight.SpotAngle = 0.85f;    
    flashlight.Attenuation = { 0.05f, 0.08f, 0.0f };
    m_lights.push_back(flashlight);
    m_flashlightIndex = static_cast<int>(m_lights.size()) - 1;


    // --- 最終処理 ---
    m_lightBuffer.NumLights = static_cast<int>(m_lights.size());
    if (m_lightBuffer.NumLights > 32) {
        m_lightBuffer.NumLights = 32;
    }
    for (int i = 0; i < m_lightBuffer.NumLights; ++i) {
        m_lightBuffer.Lights[i] = m_lights[i];
    }

    float mazeWorldWidth = maze_width * pathWidth;
    float mazeWorldHeight = maze_height * pathWidth;
    DirectX::XMVECTOR lightPos = DirectX::XMVectorSet(mazeWorldWidth / 2.0f, 50.0f, mazeWorldHeight / 2.0f, 1.0f);
    DirectX::XMVECTOR lightLookAt = DirectX::XMVectorSet(mazeWorldWidth / 2.0f, 0.0f, mazeWorldHeight / 2.0f, 1.0f);
    DirectX::XMVECTOR lightUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    m_lightViewMatrix = DirectX::XMMatrixLookAtLH(lightPos, lightLookAt, lightUp);
    m_lightProjectionMatrix = DirectX::XMMatrixOrthographicLH(mazeWorldWidth, mazeWorldHeight, 0.1f, 100.0f);
}

void LightManager::Update(float deltaTime, const DirectX::XMFLOAT3& cameraPosition, const DirectX::XMFLOAT3& cameraRotation)
{
    m_lightBuffer.CameraPosition = cameraPosition;
    UpdateFlashlight(deltaTime, cameraPosition, cameraRotation);
    m_lightBuffer.NumLights = static_cast<int>(m_lights.size());
    if (m_lightBuffer.NumLights > 32)
    {
        m_lightBuffer.NumLights = 32;
    }

    for (int i = 0; i < m_lightBuffer.NumLights; ++i)
    {
        m_lightBuffer.Lights[i] = m_lights[i];
    }
}

void LightManager::UpdateFlashlight(float deltaTime, const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& rotation)
{
    if (m_flashlightIndex == -1) return;

    ApplyFlicker(m_flashlightIndex, deltaTime);

    m_lights[m_flashlightIndex].Position = position;
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(
        rotation.x * (DirectX::XM_PI / 180.0f),
        rotation.y * (DirectX::XM_PI / 180.0f),
        rotation.z * (DirectX::XM_PI / 180.0f)
    );
    DirectX::XMVECTOR forward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    DirectX::XMVECTOR direction = DirectX::XMVector3TransformNormal(forward, rotationMatrix);
    DirectX::XMStoreFloat3(&m_lights[m_flashlightIndex].Direction, direction);
}

void LightManager::ApplyFlicker(int lightIndex, float deltaTime)
{
    m_flickerTimer += deltaTime;

    if (m_isFlickering)
    {
        // ちらつき中
        if (m_flickerTimer > 0.1f) // 0.1秒間ちらつく
        {
            m_isFlickering = false;
            m_flickerTimer = 0.0f;
            m_lights[lightIndex].Intensity = m_flashlightBaseIntensity; // 通常の明るさに戻す
        }
        else
        {
            // ランダムに明るさを変える (基準より暗く)
            std::uniform_real_distribution<float> intensityDist(m_flashlightBaseIntensity * 0.1f, m_flashlightBaseIntensity * 0.5f);
            m_lights[lightIndex].Intensity = intensityDist(m_rng);
        }
    }
    else
    {
        // 次のちらつきまでの待機中
        if (m_flickerTimer > m_nextFlickerTime)
        {
            m_isFlickering = true;
            m_flickerTimer = 0.0f;
            // 次のちらつきまでの時間をランダムに設定 (2秒～10秒後)
            std::uniform_real_distribution<float> timeDist(2.0f, 10.0f);
            m_nextFlickerTime = timeDist(m_rng);
        }
    }
}


int LightManager::AddPointLight(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT4& color, float range, float intensity)
{
    if (m_lights.size() >= 32)
    {
        return -1;
    }
    Light newLight = {};
    newLight.Enabled = true;
    newLight.Type = PointLight;
    newLight.Position = position;
    newLight.Color = color;
    newLight.Range = range;
    newLight.Intensity = intensity;
    newLight.Attenuation = { 0.2f, 0.4f, 0.1f };
    m_lights.push_back(newLight);
    return static_cast<int>(m_lights.size()) - 1;
}

void LightManager::SetLightEnabled(int index, bool enabled)
{
    if (index >= 0 && index < m_lights.size())
    {
        m_lights[index].Enabled = enabled;
    }
}

int LightManager::GetFlashlightIndex() const
{
    return m_flashlightIndex;
}