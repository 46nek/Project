#include "CameraDirector.h"
#include "Game.h"
#include "Easing.h"
#include "Stage.h" 
#include <cmath>

extern Game* g_game;

namespace {
    constexpr float VIGNETTE_NORMAL = 1.1f;
    constexpr float VIGNETTE_DASH = 1.5f; // 繝繝・す繝･荳ｭ縺ｮ貍泌・逕ｨ

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

    // 繧ｪ繝ｼ繝励ル繝ｳ繧ｰ荳ｭ縺ｯ繝薙ロ繝・ヨ縺ｪ縺励°繧蛾壼ｸｸ縺ｸ驕ｷ遘ｻ
    m_vignetteIntensity = t * VIGNETTE_NORMAL;
}

void CameraDirector::UpdateGameplay(float deltaTime, Player* player) {
    if (!player) { return; }

    // 險ｭ螳壹ｒ蜿門ｾ・
    auto& settings = g_game->GetSettings();

    // 1. 蝓ｺ譛ｬ縺ｨ縺ｪ繧玖ｦ夜㍽隗・(90蠎ｦ)
    float targetFovDeg = 45.0f;

    // 2. 繧ｹ繧ｭ繝ｫ菴ｿ逕ｨ荳ｭ縺ｮFOV繝悶・繧ｹ繝郁ｨ育ｮ・
    if (player->IsSkillActive()) {
        float boost = 0.0f;
        // 險ｭ螳・fovIntensity)縺ｫ蠢懊§縺ｦ襍ｰ縺｣縺ｦ縺・ｋ譎ゅ・霑ｽ蜉隕夜㍽隗偵ｒ螟峨∴繧・
        if (settings.fovIntensity == 1)      boost = 20.0f; // 蠑ｱ繧・
        else if (settings.fovIntensity == 2) boost = 45.0f; // 騾壼ｸｸ
        // 0 (NONE) 縺ｮ蝣ｴ蜷医・ boost = 0.0f 縺ｮ縺ｾ縺ｾ

        targetFovDeg += boost;
        m_vignetteIntensity = VIGNETTE_DASH;
    }
    else {
        m_vignetteIntensity = VIGNETTE_NORMAL;
    }

    // 繧ｫ繝｡繝ｩ縺ｫ逶ｮ讓僥OV繧定ｨｭ螳・(蠎ｦ謨ｰ豕輔°繧峨Λ繧ｸ繧｢繝ｳ縺ｫ螟画鋤)
    if (m_camera) {
        m_camera->SetTargetFOV(DirectX::XMConvertToRadians(targetFovDeg));
    }

    // 3. 菴咲ｽｮ繝ｻ蝗櫁ｻ｢蜷梧悄
    DirectX::XMFLOAT3 playerPos = player->GetPosition();
    DirectX::XMFLOAT3 playerRot = player->GetRotation();
    m_camera->SetPosition(playerPos.x, playerPos.y, playerPos.z);
    m_camera->SetRotation(playerRot.x, playerRot.y, playerRot.z);

    // 4. 繝懊ン繝ｳ繧ｰ・域昭繧鯉ｼ・
    const BobbingParams& bp = player->IsSkillActive() ? BOB_RUN : BOB_WALK;
    m_camera->SetBobbingParameters(bp.speed, bp.amount, bp.swaySpeed, bp.swayAmount, bp.rollSpeed);
    m_camera->UpdateBobbing(deltaTime, player->IsMoving());

    m_camera->Update(deltaTime);
    m_vignetteIntensity = player->IsSkillActive() ? VIGNETTE_DASH : VIGNETTE_NORMAL;

    // 繧ｪ繝ｼ繝匁園謖∵焚縺ｫ繧医ｋ隕也阜謔ｪ蛹・
    int heldOrbs = player->GetHeldOrbCount();
    if (heldOrbs > 5) {
        // 5蛟九ｒ雜・∴繧九→蟆代＠縺壹▽繝薙ロ繝・ヨ繧貞ｼｷ縺上☆繧・
        m_vignetteIntensity += (heldOrbs - 5) * 0.2f;
    }
}
