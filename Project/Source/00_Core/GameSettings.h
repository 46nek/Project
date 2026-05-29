#pragma once

// 設定値を管理する構造体
struct GameSettings {
    float volume = 0.5f;           // 0.0～1.0 の中央
    bool motionBlur = true;
    float brightness = 1.0f;       // 0.5～1.5 の中央
    int fovIntensity = 2;
    float mouseSensitivity = 1.25f; // 0.5～2.0 の中央
};
