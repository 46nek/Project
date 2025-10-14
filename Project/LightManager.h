// LightManager.h

#pragma once
#include <DirectXMath.h>
#include <vector>
#include <random>
#include "Light.h"
#include "MazeGenerator.h"

struct LightBufferType
{
    Light               Lights[MAX_LIGHTS];
    int                 NumLights;
    DirectX::XMFLOAT3   CameraPosition;
};

class LightManager
{
public:
    LightManager();
    ~LightManager();

    void Initialize(const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth, float wallHeight);
    void Update(float deltaTime, const DirectX::XMFLOAT3& cameraPosition, const DirectX::XMFLOAT3& cameraRotation);
    int AddPointLight(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT4& color, float range, float intensity);
    void SetLightEnabled(int index, bool enabled);
    int GetFlashlightIndex() const;

    const LightBufferType& GetLightBuffer() const { return m_lightBuffer; }
    DirectX::XMMATRIX GetLightViewMatrix() const { return m_lightViewMatrix; }
    DirectX::XMMATRIX GetLightProjectionMatrix() const { return m_lightProjectionMatrix; }

private:
    void UpdateFlashlight(float deltaTime, const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& rotation);
    void ApplyFlicker(int lightIndex, float deltaTime);

    std::vector<Light> m_lights;
    LightBufferType m_lightBuffer;

    DirectX::XMMATRIX m_lightViewMatrix;
    DirectX::XMMATRIX m_lightProjectionMatrix;

    int m_flashlightIndex;
    float m_flashlightBaseIntensity; 
    float m_flickerTimer;
    float m_nextFlickerTime;
    bool m_isFlickering;
    std::mt19937 m_rng;
};