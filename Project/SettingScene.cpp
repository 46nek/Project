#include "SettingScene.h"
#include "Game.h"
#include <algorithm>

// グローバル変数のGameインスタンスを参照
extern Game* g_game;

SettingScene::SettingScene()
    : m_fontFactory(nullptr), m_font(nullptr), m_selectedItem(0),
    m_isDraggingVolume(false), m_isDraggingBright(false) {
}

SettingScene::~SettingScene() {}

bool SettingScene::Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine) {
    m_graphicsDevice = graphicsDevice;
    m_input = input;
    m_audioEngine = audioEngine;

    m_input->SetCursorLock(false);
    m_input->SetCursorVisible(true);

    HRESULT hr = FW1CreateFactory(FW1_VERSION, &m_fontFactory);
    if (FAILED(hr)) return false;
    return SUCCEEDED(m_fontFactory->CreateFontWrapper(graphicsDevice->GetDevice(), L"Impact", &m_font));
}

void SettingScene::Shutdown() {
    if (m_font) m_font->Release();
    if (m_fontFactory) m_fontFactory->Release();
}

void SettingScene::UpdateValue(int dir) {
    if (m_selectedItem == 1) m_settings.motionBlur = !m_settings.motionBlur;
    if (m_selectedItem == 2) m_settings.brightness = std::clamp(m_settings.brightness + dir * 0.1f, 0.5f, 1.5f);
    if (m_selectedItem == 3) m_settings.fovIntensity = (m_settings.fovIntensity + dir + 3) % 3;
}

void SettingScene::Update(float deltaTime) {
    // --- 座標変換 (ウィンドウサイズ変更対応) ---
    int rawMx, rawMy;
    m_input->GetMousePosition(rawMx, rawMy);

    HWND hwnd = g_game->GetWindow()->GetHwnd();
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    float actualWidth = (float)(clientRect.right - clientRect.left);
    float actualHeight = (float)(clientRect.bottom - clientRect.top);
    if (actualWidth <= 0) actualWidth = 1.0f;
    if (actualHeight <= 0) actualHeight = 1.0f;

    float mx = rawMx * (static_cast<float>(Game::SCREEN_WIDTH) / actualWidth);
    float my = rawMy * (static_cast<float>(Game::SCREEN_HEIGHT) / actualHeight);

    bool isLeftClicked = m_input->IsKeyPressed(VK_LBUTTON);
    bool isLeftDown = m_input->IsKeyDown(VK_LBUTTON);

    // --- 中央集約レイアウト定数 ---
    const float centerX = Game::SCREEN_WIDTH / 2.0f;
    const float sliderW = 200.0f;
    const float sliderY = 215.0f;

    // スライダーを中央線に近づける (左右に30pxの余白)
    const float volX = centerX - sliderW - 30.0f;
    const float brightX = centerX + 30.0f;

    // 矢印項目の配置 (ラベルを左、操作を右に寄せ、中央に集める)
    const float arrowItemY = 380.0f;
    const float leftArrowX = centerX + 20.0f;
    const float rightArrowX = centerX + 180.0f;

    // --- 音量操作 ---
    if (isLeftClicked && mx >= volX && mx <= volX + sliderW &&
        my >= sliderY - 15 && my <= sliderY + 35) {
        m_isDraggingVolume = true;
    }
    if (!isLeftDown) m_isDraggingVolume = false;
    if (m_isDraggingVolume) {
        float norm = std::clamp((mx - volX) / sliderW, 0.0f, 1.0f);
        m_settings.volume = norm;
        if (m_audioEngine) m_audioEngine->SetMasterVolume(m_settings.volume);
    }

    // --- 明るさ操作 ---
    if (isLeftClicked && mx >= brightX && mx <= brightX + sliderW &&
        my >= sliderY - 15 && my <= sliderY + 35) {
        m_isDraggingBright = true;
    }
    if (!isLeftDown) m_isDraggingBright = false;
    if (m_isDraggingBright) {
        float norm = std::clamp((mx - brightX) / sliderW, 0.0f, 1.0f);
        m_settings.brightness = 0.5f + norm * 1.0f;
    }

    // --- ボタン判定 ---
    if (isLeftClicked) {
        // MOTION BLUR 行
        if (my >= arrowItemY && my <= arrowItemY + 40) {
            if (mx >= leftArrowX - 10 && mx <= leftArrowX + 40) { m_selectedItem = 1; UpdateValue(-1); }
            if (mx >= rightArrowX - 10 && mx <= rightArrowX + 40) { m_selectedItem = 1; UpdateValue(1); }
        }
        // FOV 行
        float fovY = arrowItemY + 80.0f;
        if (my >= fovY && my <= fovY + 40) {
            if (mx >= leftArrowX - 10 && mx <= leftArrowX + 40) { m_selectedItem = 3; UpdateValue(-1); }
            if (mx >= rightArrowX - 10 && mx <= rightArrowX + 40) { m_selectedItem = 3; UpdateValue(1); }
        }
        // BACKボタン (左下)
        if (mx >= 80 && mx <= 230 && my >= 600 - 10 && my <= 600 + 40) {
            m_nextScene = SceneState::Title;
        }
    }
}

void SettingScene::Render() {
    m_graphicsDevice->BeginScene(0.05f, 0.05f, 0.05f, 1.0f);
    if (m_font) {
        // SETTINGS タイトル (左上)
        m_font->DrawString(m_graphicsDevice->GetDeviceContext(), L"SETTINGS", 60.0f, 80.0f, 80.0f, 0xFFFFFFFF, FW1_RESTORESTATE);

        const float centerX = Game::SCREEN_WIDTH / 2.0f;
        const float sliderW = 200.0f;
        const float sliderY = 215.0f;
        const float volX = centerX - sliderW - 30.0f;
        const float brightX = centerX + 30.0f;

        auto DrawSlider = [&](const std::wstring& label, float x, float y, float value, float minVal, float maxVal) {
            m_font->DrawString(m_graphicsDevice->GetDeviceContext(), label.c_str(), 24.0f, x, y - 35.0f, 0xFFFFFFFF, FW1_RESTORESTATE);
            m_font->DrawString(m_graphicsDevice->GetDeviceContext(), L"----------------------", 32.0f, x, y, 0xFF444444, FW1_RESTORESTATE);
            float norm = (value - minVal) / (maxVal - minVal);
            m_font->DrawString(m_graphicsDevice->GetDeviceContext(), L"I", 32.0f, x + (sliderW * norm), y, 0xFF00FFFF, FW1_RESTORESTATE);
            };

        DrawSlider(L"VOLUME", volX, sliderY, m_settings.volume, 0.0f, 1.0f);
        DrawSlider(L"BRIGHTNESS", brightX, sliderY, m_settings.brightness, 0.5f, 1.5f);

        const float arrowItemY = 380.0f;
        const float labelX = centerX - 250.0f; // ラベルの開始位置
        const float leftArrowX = centerX + 20.0f;
        const float valueX = centerX + 75.0f;
        const float rightArrowX = centerX + 180.0f;

        auto DrawArrowItem = [&](const std::wstring& label, const std::wstring& val, float y) {
            m_font->DrawString(m_graphicsDevice->GetDeviceContext(), label.c_str(), 32.0f, labelX, y, 0xFFFFFFFF, FW1_RESTORESTATE);
            m_font->DrawString(m_graphicsDevice->GetDeviceContext(), L"◀", 32.0f, leftArrowX, y, 0xFF00FFFF, FW1_RESTORESTATE);
            m_font->DrawString(m_graphicsDevice->GetDeviceContext(), val.c_str(), 32.0f, valueX, y, 0xFFFFFFFF, FW1_RESTORESTATE);
            m_font->DrawString(m_graphicsDevice->GetDeviceContext(), L"▶", 32.0f, rightArrowX, y, 0xFF00FFFF, FW1_RESTORESTATE);
            };

        DrawArrowItem(L"MOTION BLUR", m_settings.motionBlur ? L"ON" : L"OFF", arrowItemY);
        DrawArrowItem(L"FOV INTENSITY", (m_settings.fovIntensity == 0 ? L"NONE" : m_settings.fovIntensity == 1 ? L"WEAK" : L"NORMAL"), arrowItemY + 80.0f);

        // BACKボタン
        m_font->DrawString(m_graphicsDevice->GetDeviceContext(), L"<- BACK", 32.0f, 80.0f, 600.0f, 0xFF888888, FW1_RESTORESTATE);
    }
    m_graphicsDevice->EndScene();
}