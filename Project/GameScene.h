#pragma once
#include "Scene.h"
#include <memory>
#include <vector>
#include "Camera.h"
#include "Player.h"
#include "UI.h"
#include "GameObjectManager.h"
#include "CameraDirector.h"
#include "GameEnvironment.h"

class GameScene : public Scene {
public:
    GameScene();
    ~GameScene();

    static std::unique_ptr<GameScene> s_transferInstance;

    bool Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine) override;
        
    bool InitializeEnvironment(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine);
    bool InitializeUI();
    bool InitializeGameObjects();
    bool InitializeAudio();

    void Shutdown() override;
    void Update(float deltaTime) override;
    void Render() override;

    void UpdateTitleLoop(float deltaTime);
    void RenderStageOnly();
    void SetCameraForTitle();
    void BeginOpening();

private:
    static constexpr float PLAYER_HEIGHT = 3.0f;
    static constexpr float UI_FADE_DURATION = 0.5f;

    std::unique_ptr<DirectX::SoundEffect> m_collectSound;
    std::unique_ptr<DirectX::SoundEffect> m_walkSoundEffect;
    std::unique_ptr<DirectX::SoundEffect> m_runSoundEffect;

    std::unique_ptr<GameEnvironment> m_environment;

    std::shared_ptr<Camera> m_camera;

    std::unique_ptr<Player> m_player;
    std::unique_ptr<UI> m_ui;

    std::unique_ptr<GameObjectManager> m_gameObjectManager;
    std::unique_ptr<CameraDirector> m_cameraDirector;

    std::vector<Model*> m_cachedDynamicModels;

    float m_uiFadeTimer;
};