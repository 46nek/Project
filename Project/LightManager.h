#pragma once
#include <vector>
#include <DirectXMath.h>

enum LightType
{
    DirectionalLight = 0,
    PointLight = 1,
    SpotLight = 2
};

struct Light
{
    DirectX::XMFLOAT4   Color;
    DirectX::XMFLOAT3   Direction;
    float               SpotAngle;
    DirectX::XMFLOAT3   Position;
    float               Range;
    DirectX::XMFLOAT3   Attenuation;
    int                 Type;
    BOOL                Enabled;
    float               Intensity;
    DirectX::XMFLOAT2   Padding;
};

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
    void Update(float deltaTime, const DirectX::XMFLOAT3& playerPosition);

    const LightBufferType& GetLightBuffer() const { return m_lightBuffer; }
    DirectX::XMMATRIX GetLightViewMatrix() const { return m_lightViewMatrix; }
    DirectX::XMMATRIX GetLightProjectionMatrix() const { return m_lightProjectionMatrix; }

private:
    std::vector<Light> m_lights;
    LightBufferType m_lightBuffer;
    float m_flickerTimer;
    DirectX::XMMATRIX m_lightViewMatrix;
    DirectX::XMMATRIX m_lightProjectionMatrix;
};