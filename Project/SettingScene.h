#pragma once
#include "Scene.h"
#include <vector>
#include <string>
#include "FW1FontWrapper.h"

struct GameSettings {
    float volume = 0.5f;
    bool motionBlur = true;
    float brightness = 1.0f;
    int fovIntensity = 2; // 0:なし, 1:弱め, 2:通常
};

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
    int m_selectedItem; // 0:Volume, 1:Blur, 2:Bright, 3:FOV, 4:BACK

    // シークバー操作用
    bool m_isDraggingVolume;
    float m_sliderX, m_sliderY, m_sliderWidth;

    void UpdateValue(int direction);
    void HandleVolumeInput(); // 数字キー入力
    std::wstring GetFovText();
};