#pragma once
#include "Scene.h"
#include <vector>
#include <string>
#include "FW1FontWrapper.h"
#include "GameSettings.h"

class SettingScene : public Scene {
public:
    SettingScene();
    ~SettingScene();

    bool Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine) override;
    void Shutdown() override;
    void Update(float deltaTime) override;
    void Render() override;

private:
    IFW1Factory* m_fontFactory;
    IFW1FontWrapper* m_font;

    GameSettings m_settings;
    int m_selectedItem;

    // シークバー操作用
    bool m_isDraggingVolume;
    bool m_isDraggingBright; 
    bool m_isDraggingSens; 

    void UpdateValue(int direction);
    std::wstring GetFovText();

    // レイアウト定数
    const float SLIDER_WIDTH = 250.0f;
    const float VOL_X = 200.0f;
    const float BRIGHT_X = 650.0f;
    const float SLIDER_Y = 215.0f;
    const float ARROW_Y_START = 350.0f;
    const float BACK_X = 100.0f;
    const float BACK_Y = 600.0f;
    const float SENS_X = 425.0f; 
    const float SENS_Y = 300.0f; 
};