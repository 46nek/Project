#pragma once
#include <DirectXMath.h>
#include <vector>
#include "Light.h"

/**
 * @struct LightBufferType
 * @brief シェーダーに渡すライト情報全体を格納する構造体
 */
struct LightBufferType
{
    Light               Lights[16];
    int                 NumLights;
    DirectX::XMFLOAT3   CameraPosition;
};

/**
 * @class LightManager
 * @brief シーン内の全てのライトを管理し、シェーダー用のバッファを更新
 */
class LightManager
{
public:
    LightManager();
    ~LightManager();

    void Initialize();
    void Update(float deltaTime, const DirectX::XMFLOAT3& playerPosition, const DirectX::XMFLOAT3& cameraRotation);

    // ゲッター
    const LightBufferType& GetLightBuffer() const { return m_lightBuffer; }
    DirectX::XMMATRIX GetLightViewMatrix() const { return m_lightViewMatrix; }
    DirectX::XMMATRIX GetLightProjectionMatrix() const { return m_lightProjectionMatrix; }

private:
    std::vector<Light> m_lights;
    LightBufferType m_lightBuffer;
    float m_flickerTimer;
    float m_originalIntensity;

    // シャドウマッピング用
    DirectX::XMMATRIX m_lightViewMatrix;
    DirectX::XMMATRIX m_lightProjectionMatrix;
};