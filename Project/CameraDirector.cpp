#include "CameraDirector.h"
#include "Game.h"
#include "Easing.h"
#include "Stage.h" // ★cpp側でインクルード
#include <cmath>

namespace {
    constexpr float VIGNETTE_MIN_INTENSITY = 1.1f;
    constexpr float VIGNETTE_MAX_INTENSITY = 2.0f;
    constexpr float VIGNETTE_WARNING_THRESHOLD = 0.3f;

    struct BobbingParams {
        float speed, amount, swaySpeed, swayAmount, rollSpeed;
    };
    constexpr BobbingParams BOB_WALK = { 14.0f, 0.03f, 7.0f, 0.05f, 7.0f };
    constexpr BobbingParams BOB_RUN = { 18.0f, 0.05f, 10.0f, 0.08f, 9.0f };
}

CameraDirector::CameraDirector(std::shared_ptr<Camera> camera)
    : m_camera(camera),
    m_isOpening(false),
    m_openingTimer(0.0f),
    m_openingDuration(2.5f),
    m_titleTimer(0.0f),
    m_vignetteIntensity(0.0f),
    m_titleCamPos(0, 0, 0),
    m_titleCamRot(0, 0, 0),
    m_startCamPos(0, 0, 0),
    m_startCamRot(0, 0, 0)
{
}

void CameraDirector::Initialize() {
    m_vignetteIntensity = 0.0f;
    m_isOpening = false;
}

void CameraDirector::SetCameraForTitle(Stage* stage) {
    if (!stage || !m_camera) return;

    std::pair<int, int> startPos = stage->GetStartPosition();
    float pathWidth = stage->GetPathWidth();
    float centerX = (static_cast<float>(startPos.first) + 0.5f) * pathWidth;

    float titleX = centerX;
    float titleY = 5.0f;
    float titleZ = 4.5f * pathWidth;

    // ★【修正2】{} での代入エラーを防ぐため、コンストラクタを使用
    m_titleCamPos = DirectX::XMFLOAT3(titleX, titleY, titleZ);
    m_titleCamRot = DirectX::XMFLOAT3(0.0f, 180.0f, 0.0f);

    m_camera->SetPosition(m_titleCamPos.x, m_titleCamPos.y, m_titleCamPos.z);
    m_camera->SetRotation(m_titleCamRot.x, m_titleCamRot.y, m_titleCamRot.z);

    // 初回更新（ここでPrev行列とCurrent行列を同期させる）
    m_camera->Update(0.0f);
    m_camera->Update(0.0f);

    UpdateTitleLoop(0.0f);
}

void CameraDirector::BeginOpening(const DirectX::XMFLOAT3& startPos, const DirectX::XMFLOAT3& startRot) {
    m_isOpening = true;
    m_openingTimer = 0.0f;
    m_startCamPos = startPos;
    m_startCamRot = startRot;

    if (m_camera) {
        m_camera->SetPosition(m_titleCamPos.x, m_titleCamPos.y, m_titleCamPos.z);
        m_camera->SetRotation(m_titleCamRot.x, m_titleCamRot.y, m_titleCamRot.z);
    }
}

void CameraDirector::Update(float deltaTime, Player* player, bool isTitle) {
    if (isTitle) {
        UpdateTitleLoop(deltaTime);
    }
    else if (m_isOpening) {
        UpdateOpening(deltaTime);
    }
    else {
        UpdateGameplay(deltaTime, player);
    }
}

void CameraDirector::UpdateTitleLoop(float deltaTime) {
    m_titleTimer += deltaTime;

    // ★【修正3】モーションブラー対策
    // カメラ位置が固定でも、Updateを呼ばないと「前回の位置」が更新されず
    // 「ものすごい速度で移動し続けている」と判定され、画面が激しくブレます。
    if (m_camera) {
        m_camera->Update(deltaTime);
    }
}

void CameraDirector::UpdateOpening(float deltaTime) {
    m_openingTimer += deltaTime;
    float t = m_openingTimer / m_openingDuration;

    if (t >= 1.0f) {
        t = 1.0f;
        m_isOpening = false;
    }

    float easeT = Easing::GetValue(EasingType::EaseInOutExpo, t);

    float x = m_titleCamPos.x + (m_startCamPos.x - m_titleCamPos.x) * easeT;
    float y = m_titleCamPos.y + (m_startCamPos.y - m_titleCamPos.y) * easeT;
    float z = m_titleCamPos.z + (m_startCamPos.z - m_titleCamPos.z) * easeT;

    float currentYaw = m_titleCamRot.y;
    float targetYaw = m_startCamRot.y;

    if (targetYaw - currentYaw > 180.0f) currentYaw += 360.0f;
    if (targetYaw - currentYaw < -180.0f) currentYaw -= 360.0f;

    float rx = m_titleCamRot.x + (m_startCamRot.x - m_titleCamRot.x) * easeT;
    float ry = currentYaw + (targetYaw - currentYaw) * easeT;
    float rz = m_titleCamRot.z + (m_startCamRot.z - m_titleCamRot.z) * easeT;

    m_camera->SetPosition(x, y, z);
    m_camera->SetRotation(rx, ry, rz);
    m_camera->Update(deltaTime);

    m_vignetteIntensity = t * VIGNETTE_MIN_INTENSITY;
}

void CameraDirector::UpdateGameplay(float deltaTime, Player* player) {
    if (!player) return;

    // FOV制御
    float baseFov = DirectX::XM_PI / 4.0f;
    float runFov = DirectX::XM_PI / 3.0f;
    if (player->IsRunning()) {
        m_camera->SetTargetFOV(runFov);
    }
    else {
        m_camera->SetTargetFOV(baseFov);
    }

    // 位置・回転同期
    DirectX::XMFLOAT3 playerPos = player->GetPosition();
    DirectX::XMFLOAT3 playerRot = player->GetRotation();
    m_camera->SetPosition(playerPos.x, playerPos.y, playerPos.z);
    m_camera->SetRotation(playerRot.x, playerRot.y, playerRot.z);

    // ボビング（揺れ）
    const BobbingParams& bp = player->IsRunning() ? BOB_RUN : BOB_WALK;
    m_camera->SetBobbingParameters(bp.speed, bp.amount, bp.swaySpeed, bp.swayAmount, bp.rollSpeed);
    m_camera->UpdateBobbing(deltaTime, player->IsMoving());

    m_camera->Update(deltaTime);

    // ビネット効果更新
    UpdateVignette(player->GetStaminaPercentage());
}

void CameraDirector::UpdateVignette(float staminaPercentage) {
    if (staminaPercentage > VIGNETTE_WARNING_THRESHOLD) {
        m_vignetteIntensity = VIGNETTE_MIN_INTENSITY;
    }
    else {
        float factorInWarningZone = staminaPercentage / VIGNETTE_WARNING_THRESHOLD;
        float intensityFactor = 1.0f - factorInWarningZone;
        m_vignetteIntensity = VIGNETTE_MIN_INTENSITY + (VIGNETTE_MAX_INTENSITY - VIGNETTE_MIN_INTENSITY) * intensityFactor;
    }
}