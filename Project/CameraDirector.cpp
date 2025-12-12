#include "CameraDirector.h"
#include "Game.h"
#include "Easing.h"
#include "Stage.h" 
#include <cmath>

namespace {
    constexpr float VIGNETTE_NORMAL = 1.1f;
    constexpr float VIGNETTE_DASH = 1.5f; // ダッシュ中の演出用

    struct BobbingParams {
        float speed, amount, swaySpeed, swayAmount, rollSpeed;
    };
    constexpr BobbingParams BOB_WALK = { 14.0f, 0.03f, 7.0f, 0.05f, 7.0f };
    constexpr BobbingParams BOB_RUN = { 22.0f, 0.06f, 12.0f, 0.1f, 10.0f };
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
    m_startCamRot(0, 0, 0) {
}

void CameraDirector::Initialize() {
    m_vignetteIntensity = 0.0f;
    m_isOpening = false;
}

void CameraDirector::SetCameraForTitle(Stage* stage) {
    if (!stage || !m_camera) { return; }

    std::pair<int, int> startPos = stage->GetStartPosition();
    float pathWidth = stage->GetPathWidth();
    float centerX = (static_cast<float>(startPos.first) + 0.5f) * pathWidth;

    float titleX = centerX;
    float titleY = 5.0f;
    float titleZ = 4.5f * pathWidth;

    m_titleCamPos = DirectX::XMFLOAT3(titleX, titleY, titleZ);
    m_titleCamRot = DirectX::XMFLOAT3(0.0f, 180.0f, 0.0f);

    m_camera->SetPosition(m_titleCamPos.x, m_titleCamPos.y, m_titleCamPos.z);
    m_camera->SetRotation(m_titleCamRot.x, m_titleCamRot.y, m_titleCamRot.z);

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

    // オープニング中はビネットなしから通常へ遷移
    m_vignetteIntensity = t * VIGNETTE_NORMAL;
}

void CameraDirector::UpdateGameplay(float deltaTime, Player* player) {
    if (!player) { return; }

    // FOV制御
    float baseFov = DirectX::XM_PI / 4.0f;
    float runFov = DirectX::XM_PI / 2.0f; // 疾走感を出すために広角にする

    if (player->IsSkillActive()) {
        m_camera->SetTargetFOV(runFov);
        // ダッシュ中はビネットを少し強めて集中線のような効果を狙う
        m_vignetteIntensity = VIGNETTE_DASH;
    }
    else {
        m_camera->SetTargetFOV(baseFov);
        m_vignetteIntensity = VIGNETTE_NORMAL;
    }

    // 位置・回転同期
    DirectX::XMFLOAT3 playerPos = player->GetPosition();
    DirectX::XMFLOAT3 playerRot = player->GetRotation();
    m_camera->SetPosition(playerPos.x, playerPos.y, playerPos.z);
    m_camera->SetRotation(playerRot.x, playerRot.y, playerRot.z);

    // ボビング（揺れ）
    const BobbingParams& bp = player->IsSkillActive() ? BOB_RUN : BOB_WALK;
    m_camera->SetBobbingParameters(bp.speed, bp.amount, bp.swaySpeed, bp.swayAmount, bp.rollSpeed);
    m_camera->UpdateBobbing(deltaTime, player->IsMoving());

    m_camera->Update(deltaTime);
}
