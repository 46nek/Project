#include "SettingScene.h"
#include "Game.h"
#include <algorithm>

extern Game* g_game;

SettingScene::SettingScene()
    : m_fontFactory(nullptr), m_font(nullptr), m_selectedItem(0),
    m_isDraggingVolume(false), m_isDraggingBright(false), m_isDraggingSens(false) { // 追加
}

SettingScene::~SettingScene() {}

bool SettingScene::Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine) {
    m_graphicsDevice = graphicsDevice;
    m_input = input;
    m_audioEngine = audioEngine;

    m_settings = g_game->GetSettings();

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
    if (m_selectedItem == 2) m_settings.brightness = std::clamp(m_settings.brightness + dir * 0.5f, 0.5f, 1.5f);
    if (m_selectedItem == 3) m_settings.fovIntensity = (m_settings.fovIntensity + dir + 3) % 3;

    g_game->GetSettings() = m_settings;
}

void SettingScene::Update(float deltaTime) {
    //座標変換
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

    const float centerX = Game::SCREEN_WIDTH / 2.0f;
    const float sliderW = 200.0f;
    const float sliderX = centerX - 100.0f; // 縦に並べるためX座標を統一
    const float volY = 200.0f;
    const float brightY = 280.0f;
    const float sensY = 360.0f;

    auto UpdateSliderLogic = [&](float x, float y, float& value, float minVal, float maxVal, bool& dragging) {
        if (isLeftClicked && mx >= x && mx <= x + sliderW && my >= y - 15 && my <= y + 35) dragging = true;
        if (!isLeftDown) dragging = false;
        if (dragging) {
            float norm = std::clamp((mx - x) / sliderW, 0.0f, 1.0f);
            int displayVal = static_cast<int>(std::round(norm * 100.0f)); // 0～100の1刻み
            value = minVal + (displayVal / 100.0f) * (maxVal - minVal);
            return true;
        }
        return false;
        };

    if (UpdateSliderLogic(sliderX, volY, m_settings.volume, 0.0f, 1.0f, m_isDraggingVolume)) {
        if (m_audioEngine) m_audioEngine->SetMasterVolume(m_settings.volume);
    }
    UpdateSliderLogic(sliderX, brightY, m_settings.brightness, 0.5f, 1.5f, m_isDraggingBright);
    UpdateSliderLogic(sliderX, sensY, m_settings.mouseSensitivity, 0.5f, 2.0f, m_isDraggingSens);

    g_game->GetSettings() = m_settings;

    const float arrowItemY = 480.0f;
    const float leftArrowX = centerX + 20.0f;
    const float rightArrowX = centerX + 180.0f;

    // ボタン判定
    if (isLeftClicked) {
        if (my >= arrowItemY && my <= arrowItemY + 40) {
            if (mx >= leftArrowX - 10 && mx <= leftArrowX + 40) { m_selectedItem = 1; UpdateValue(-1); }
            if (mx >= rightArrowX - 10 && mx <= rightArrowX + 40) { m_selectedItem = 1; UpdateValue(1); }
        }
        float fovY = arrowItemY + 80.0f;
        if (my >= fovY && my <= fovY + 40) {
            if (mx >= leftArrowX - 10 && mx <= leftArrowX + 40) { m_selectedItem = 3; UpdateValue(-1); }
            if (mx >= rightArrowX - 10 && mx <= rightArrowX + 40) { m_selectedItem = 3; UpdateValue(1); }
        }
        if (mx >= 80 && mx <= 230 && my >= 600 - 10 && my <= 600 + 40) {
            m_nextScene = SceneState::Title;
        }
    }
}

void SettingScene::Render() {
    m_graphicsDevice->BeginScene(0.05f, 0.05f, 0.05f, 1.0f);
    if (m_font) {
        m_font->DrawString(m_graphicsDevice->GetDeviceContext(), L"SETTINGS", 60.0f, 80.0f, 80.0f, 0xFFFFFFFF, FW1_RESTORESTATE);

        const float centerX = Game::SCREEN_WIDTH / 2.0f;
        const float sliderX = centerX - 100.0f;
        const float sliderW = 200.0f;

        auto DrawSlider = [&](const std::wstring& label, float x, float y, float value, float minVal, float maxVal) {
            m_font->DrawString(m_graphicsDevice->GetDeviceContext(), label.c_str(), 24.0f, x, y - 35.0f, 0xFFFFFFFF, FW1_RESTORESTATE);
            m_font->DrawString(m_graphicsDevice->GetDeviceContext(), L"----------------------", 32.0f, x, y, 0xFF444444, FW1_RESTORESTATE);

            float norm = (value - minVal) / (maxVal - minVal);
            int displayVal = static_cast<int>(std::round(norm * 100.0f));

            m_font->DrawString(m_graphicsDevice->GetDeviceContext(), L"I", 32.0f, x + (sliderW * norm), y, 0xFF00FFFF, FW1_RESTORESTATE);
            // 数値表示
            m_font->DrawString(m_graphicsDevice->GetDeviceContext(), std::to_wstring(displayVal).c_str(), 24.0f, x + sliderW + 20.0f, y, 0xFFFFFFFF, FW1_RESTORESTATE);
            };

        DrawSlider(L"VOLUME", sliderX, 200.0f, m_settings.volume, 0.0f, 1.0f);
        DrawSlider(L"BRIGHTNESS", sliderX, 280.0f, m_settings.brightness, 0.5f, 1.5f);
        DrawSlider(L"MOUSE SENSITIVITY", sliderX, 360.0f, m_settings.mouseSensitivity, 0.5f, 2.0f);

        const float arrowItemY = 480.0f;
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
        m_font->DrawString(m_graphicsDevice->GetDeviceContext(), L"《 BACK", 32.0f, 80.0f, 600.0f, 0xFF888888, FW1_RESTORESTATE);
    }
    m_graphicsDevice->EndScene();
}