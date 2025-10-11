#pragma once
#include <vector>
#include <DirectXMath.h>
#include "Light.h"

struct LightBufferType
{
    Light               Lights[16];
    int                 NumLights;
    DirectX::XMFLOAT3   CameraPosition;
};

class LightManager
{
public:
    LightManager();
    ~LightManager();

    void Initialize();
    void Update(float deltaTime, const DirectX::XMFLOAT3& playerPosition, const DirectX::XMFLOAT3& cameraRotation);

    const LightBufferType& GetLightBuffer() const { return m_lightBuffer; }
    DirectX::XMMATRIX GetLightViewMatrix() const { return m_lightViewMatrix; }
    DirectX::XMMATRIX GetLightProjectionMatrix() const { return m_lightProjectionMatrix; }

private:
    std::vector<Light> m_lights;
    LightBufferType m_lightBuffer;

    float m_flickerTimer;
    float m_originalIntensity;

    DirectX::XMMATRIX m_lightViewMatrix;
    DirectX::XMMATRIX m_lightProjectionMatrix;
};