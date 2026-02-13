#pragma once
#include "Camera.h"
#include "Player.h"
#include <DirectXMath.h>
#include <memory>

class Stage;

/**
 * @brief 繧ｷ繝ｼ繝ｳ縺ｮ迥ｶ諷具ｼ医ち繧､繝医Ν縲√が繝ｼ繝励ル繝ｳ繧ｰ縲√・繝ｬ繧､荳ｭ・峨↓蠢懊§縺溘き繝｡繝ｩ縺ｮ蜍輔″繧堤ｮ｡逅・☆繧九け繝ｩ繧ｹ
 */
class CameraDirector {
public:
    CameraDirector(std::shared_ptr<Camera> camera);

    void Initialize();
    void SetCameraForTitle(Stage* stage);
    void BeginOpening(const DirectX::XMFLOAT3& startPos, const DirectX::XMFLOAT3& startRot);

    void Update(float deltaTime, Player* player, bool isTitle);

    bool IsOpening() const { return m_isOpening; }
    float GetVignetteIntensity() const { return m_vignetteIntensity; }

private:
    void UpdateTitleLoop(float deltaTime);
    void UpdateOpening(float deltaTime);
    void UpdateGameplay(float deltaTime, Player* player);

    std::shared_ptr<Camera> m_camera;

    bool m_isOpening;
    float m_openingTimer;
    float m_openingDuration;
    float m_titleTimer;

    DirectX::XMFLOAT3 m_titleCamPos;
    DirectX::XMFLOAT3 m_titleCamRot;
    DirectX::XMFLOAT3 m_startCamPos;
    DirectX::XMFLOAT3 m_startCamRot;

    float m_vignetteIntensity;
};
