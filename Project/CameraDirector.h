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

    // 初期化・設定系
    void Initialize();
    void SetCameraForTitle(Stage* stage);
    void BeginOpening(const DirectX::XMFLOAT3& startPos, const DirectX::XMFLOAT3& startRot);

    // 更新処理
    // isTitle: タイトル画面かどうか
    void Update(float deltaTime, Player* player, bool isTitle);

    // ゲッター
    bool IsOpening() const { return m_isOpening; }
    float GetVignetteIntensity() const { return m_vignetteIntensity; }

private:
    void UpdateTitleLoop(float deltaTime);
    void UpdateOpening(float deltaTime);
    void UpdateGameplay(float deltaTime, Player* player);
    void UpdateVignette(float staminaPercentage);

    std::shared_ptr<Camera> m_camera;

    // 演出用変数
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