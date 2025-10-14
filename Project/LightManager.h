// LightManager.h

#pragma once
#include <DirectXMath.h>
#include <vector>
#include "Light.h"
#include "MazeGenerator.h"

/**
 * @struct LightBufferType
 * @brief シェーダーに渡すライト情報全体を格納する構造体
 */
struct LightBufferType
{
    Light               Lights[32]; // ライトの最大数を16から32に増やします
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

    void Initialize(const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth, float wallHeight);
    void Update(float deltaTime, const DirectX::XMFLOAT3& cameraPosition);

    /**
     * @brief 新しいポイントライトを追加します
     * @return 追加されたライトのインデックス。上限に達している場合は-1
     */
    int AddPointLight(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT4& color, float range, float intensity);

    /**
     * @brief 指定されたインデックスのライトの有効/無効を切り替えます
     * @param index ライトのインデックス
     * @param enabled 有効にする場合はtrue
     */
    void SetLightEnabled(int index, bool enabled);


    // ゲッター
    const LightBufferType& GetLightBuffer() const { return m_lightBuffer; }
    DirectX::XMMATRIX GetLightViewMatrix() const { return m_lightViewMatrix; }
    DirectX::XMMATRIX GetLightProjectionMatrix() const { return m_lightProjectionMatrix; }

private:
    std::vector<Light> m_lights;
    LightBufferType m_lightBuffer;

    // シャドウマッピング用
    DirectX::XMMATRIX m_lightViewMatrix;
    DirectX::XMMATRIX m_lightProjectionMatrix;
};