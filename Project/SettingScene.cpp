#include "SettingScene.h"
#include "Game.h"
#include <algorithm>

SettingScene::SettingScene()
    : m_fontFactory(nullptr), m_font(nullptr), m_selectedItem(0),
    m_isDraggingVolume(false), m_sliderX(600.0f), m_sliderY(215.0f), m_sliderWidth(300.0f) {
}

SettingScene::~SettingScene() {}

bool SettingScene::Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine) {
    m_graphicsDevice = graphicsDevice;
    m_input = input;
    m_audioEngine = audioEngine;
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
    int mx, my;
    m_input->GetMousePosition(mx, my);
    bool isLeftClicked = m_input->IsKeyPressed(VK_LBUTTON) || (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
    bool isLeftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000);

    // --- レイアウト定数 ---
    const float volumeSliderX = 200.0f;
    const float brightSliderX = 650.0f;
    const float sliderY = 215.0f;
    // 文字の幅に合わせて調整（ハイフン20個分くらいに収まる幅に短縮）
    const float sliderWidth = 250.0f;

    // --- 1. 音量：シークバー操作 ---
    if (isLeftClicked && mx >= volumeSliderX && mx <= volumeSliderX + sliderWidth &&
        my >= sliderY - 10 && my <= sliderY + 30) {
        m_isDraggingVolume = true;
    }
    if (!isLeftDown) m_isDraggingVolume = false;

    if (m_isDraggingVolume) {
        float norm = std::clamp((mx - volumeSliderX) / sliderWidth, 0.0f, 1.0f);
        m_settings.volume = norm;
        if (m_audioEngine) m_audioEngine->SetMasterVolume(m_settings.volume);
    }

    // --- 2. 明るさ：シークバー操作 ---
    static bool isDraggingBright = false;
    if (isLeftClicked && mx >= brightSliderX && mx <= brightSliderX + sliderWidth &&
        my >= sliderY - 10 && my <= sliderY + 30) {
        isDraggingBright = true;
    }
    if (!isLeftDown) isDraggingBright = false;

    if (isDraggingBright) {
        float norm = std::clamp((mx - brightSliderX) / sliderWidth, 0.0f, 1.0f);
        m_settings.brightness = 0.5f + norm * 1.0f;
    }

    // --- 3. その他のボタン判定 ---
    float itemStartY = 350.0f;
    if (isLeftClicked) {
        // MOTION BLUR (◀▶)
        if (my >= itemStartY && my <= itemStartY + 40) {
            if (mx >= 550 && mx <= 590) { m_selectedItem = 1; UpdateValue(-1); }
            if (mx >= 800 && mx <= 840) { m_selectedItem = 1; UpdateValue(1); }
        }
        // FOV INTENSITY (◀▶)
        float fovY = itemStartY + 80.0f;
        if (my >= fovY && my <= fovY + 40) {
            if (mx >= 550 && mx <= 590) { m_selectedItem = 3; UpdateValue(-1); }
            if (mx >= 800 && mx <= 840) { m_selectedItem = 3; UpdateValue(1); }
        }
        // BACKボタン
        if (mx >= 100 && mx <= 300 && my >= 600 && my <= 650) {
            m_nextScene = SceneState::Title;
        }
    }
}

void SettingScene::Render() {
    m_graphicsDevice->BeginScene(0.05f, 0.05f, 0.05f, 1.0f);
    if (m_font) {
        m_font->DrawString(m_graphicsDevice->GetDeviceContext(), L"SETTINGS", 60.0f, 100.0f, 80.0f, 0xFFFFFFFF, FW1_RESTORESTATE);

        auto DrawSlider = [&](const std::wstring& label, float x, float y, float value, float minVal, float maxVal) {
            m_font->DrawString(m_graphicsDevice->GetDeviceContext(), label.c_str(), 30.0f, x, y - 40.0f, 0xFFFFFFFF, FW1_RESTORESTATE);

            // 背景の棒を十分長く（25個）表示し、突き抜けを防ぐ
            m_font->DrawString(m_graphicsDevice->GetDeviceContext(), L"----------------------", 40.0f, x, y, 0xFF444444, FW1_RESTORESTATE);

            // つまみの移動範囲を250pxに制限
            float norm = (value - minVal) / (maxVal - minVal);
            float displayWidth = 250.0f;
            m_font->DrawString(m_graphicsDevice->GetDeviceContext(), L"I", 40.0f, x + (displayWidth * norm), y, 0xFF00FFFF, FW1_RESTORESTATE);
            };

        // 音量と明るさを横並び
        DrawSlider(L"VOLUME", 200.0f, 215.0f, m_settings.volume, 0.0f, 1.0f);
        DrawSlider(L"BRIGHTNESS", 650.0f, 215.0f, m_settings.brightness, 0.5f, 1.5f);

        auto DrawArrowItem = [&](const std::wstring& label, const std::wstring& val, float y) {
            m_font->DrawString(m_graphicsDevice->GetDeviceContext(), label.c_str(), 40.0f, 200.0f, y, 0xFFFFFFFF, FW1_RESTORESTATE);
            m_font->DrawString(m_graphicsDevice->GetDeviceContext(), L"◀", 40.0f, 550.0f, y, 0xFF00FFFF, FW1_RESTORESTATE);
            m_font->DrawString(m_graphicsDevice->GetDeviceContext(), val.c_str(), 40.0f, 620.0f, y, 0xFFFFFFFF, FW1_RESTORESTATE);
            m_font->DrawString(m_graphicsDevice->GetDeviceContext(), L"▶", 40.0f, 800.0f, y, 0xFF00FFFF, FW1_RESTORESTATE);
            };

        float arrowY = 350.0f;
        DrawArrowItem(L"MOTION BLUR", m_settings.motionBlur ? L"ON" : L"OFF", arrowY);
        DrawArrowItem(L"FOV INTENSITY", (m_settings.fovIntensity == 0 ? L"NONE" : m_settings.fovIntensity == 1 ? L"WEAK" : L"NORMAL"), arrowY + 80.0f);

        m_font->DrawString(m_graphicsDevice->GetDeviceContext(), L"<- BACK", 40.0f, 100.0f, 600.0f, 0xFF888888, FW1_RESTORESTATE);
    }
    m_graphicsDevice->EndScene();
}