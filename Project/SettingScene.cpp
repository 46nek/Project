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

void SettingScene::Update(float deltaTime) {
    int mx, my;
    m_input->GetMousePosition(mx, my);
    bool isLeftClicked = m_input->IsKeyPressed(VK_LBUTTON) || (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
    bool isLeftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000);

    // --- 1. 音量：シークバー操作 ---
    if (isLeftClicked && mx >= m_sliderX && mx <= m_sliderX + m_sliderWidth &&
        my >= m_sliderY - 10 && my <= m_sliderY + 30) {
        m_isDraggingVolume = true;
    }
    if (!isLeftDown) m_isDraggingVolume = false;

    if (m_isDraggingVolume) {
        float norm = std::clamp((mx - m_sliderX) / m_sliderWidth, 0.0f, 1.0f);
        m_settings.volume = norm;
        if (m_audioEngine) m_audioEngine->SetMasterVolume(m_settings.volume);
    }

    // --- 2. ◀▶ ボタン判定 ---
    float itemStartX = 200.0f;
    float itemStartY = 200.0f;
    float stepY = 80.0f;

    for (int i = 1; i <= 3; ++i) { // 音量以外の項目
        float currentY = itemStartY + i * stepY;
        // ◀ ボタン (座標はRenderの配置に合わせる)
        if (isLeftClicked && mx >= 550 && mx <= 590 && my >= currentY && my <= currentY + 40) {
            m_selectedItem = i; UpdateValue(-1);
        }
        // ▶ ボタン
        if (isLeftClicked && mx >= 800 && mx <= 840 && my >= currentY && my <= currentY + 40) {
            m_selectedItem = i; UpdateValue(1);
        }
    }

    // BACKボタン
    if (isLeftClicked && mx >= 100 && mx <= 300 && my >= 800 && my <= 850) {
        m_nextScene = SceneState::Title;
    }
}

void SettingScene::UpdateValue(int dir) {
    if (m_selectedItem == 1) m_settings.motionBlur = !m_settings.motionBlur;
    if (m_selectedItem == 2) m_settings.brightness = std::clamp(m_settings.brightness + dir * 0.1f, 0.5f, 1.5f);
    if (m_selectedItem == 3) m_settings.fovIntensity = (m_settings.fovIntensity + dir + 3) % 3;
}

void SettingScene::Render() {
    m_graphicsDevice->BeginScene(0.05f, 0.05f, 0.05f, 1.0f);
    if (m_font) {
        m_font->DrawString(m_graphicsDevice->GetDeviceContext(), L"SETTINGS", 60.0f, 100.0f, 80.0f, 0xFFFFFFFF, FW1_RESTORESTATE);

        // 音量描画 (シークバー形式)
        m_font->DrawString(m_graphicsDevice->GetDeviceContext(), L"VOLUME", 40.0f, 200.0f, 200.0f, 0xFFFFFFFF, FW1_RESTORESTATE);
        // バーの土台
        std::wstring bar = L"----------"; // 背景
        m_font->DrawString(m_graphicsDevice->GetDeviceContext(), bar.c_str(), 40.0f, m_sliderX, m_sliderY, 0xFF444444, FW1_RESTORESTATE);
        std::wstring indicator = L"I"; // 現在値のつまみ
        m_font->DrawString(m_graphicsDevice->GetDeviceContext(), indicator.c_str(), 40.0f, m_sliderX + (m_sliderWidth * m_settings.volume), m_sliderY, 0xFF00FFFF, FW1_RESTORESTATE);

        // その他項目
        auto DrawArrowItem = [&](int idx, const std::wstring& label, const std::wstring& val) {
            float y = 200.0f + idx * 80.0f;
            m_font->DrawString(m_graphicsDevice->GetDeviceContext(), label.c_str(), 40.0f, 200.0f, y, 0xFFFFFFFF, FW1_RESTORESTATE);
            m_font->DrawString(m_graphicsDevice->GetDeviceContext(), L"◀", 40.0f, 550.0f, y, 0xFF00FFFF, FW1_RESTORESTATE);
            m_font->DrawString(m_graphicsDevice->GetDeviceContext(), val.c_str(), 40.0f, 620.0f, y, 0xFFFFFFFF, FW1_RESTORESTATE);
            m_font->DrawString(m_graphicsDevice->GetDeviceContext(), L"▶", 40.0f, 800.0f, y, 0xFF00FFFF, FW1_RESTORESTATE);
            };

        DrawArrowItem(1, L"MOTION BLUR", m_settings.motionBlur ? L"ON" : L"OFF");
        DrawArrowItem(2, L"BRIGHTNESS", std::to_wstring(m_settings.brightness).substr(0, 3));
        DrawArrowItem(3, L"FOV INTENSITY", (m_settings.fovIntensity == 0 ? L"NONE" : m_settings.fovIntensity == 1 ? L"WEAK" : L"NORMAL"));

        m_font->DrawString(m_graphicsDevice->GetDeviceContext(), L"<- BACK", 40.0f, 100.0f, 700.0f, 0xFF888888, FW1_RESTORESTATE);
    }
    m_graphicsDevice->EndScene();
}