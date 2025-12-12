#pragma once
#include <memory>
#include <DirectXMath.h>
#include "Camera.h"
#include "Player.h"

class Stage;
// 責務: シーンの状態（タイトル、オープニング、プレイ中）に応じたカメラの動きを管理する
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