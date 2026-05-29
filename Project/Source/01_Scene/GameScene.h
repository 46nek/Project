#pragma once
#include "Source/01_Scene/Scene.h"
#include "Source/04_GamePlay/Camera.h"
#include "Source/04_GamePlay/Player.h"
#include "Source/04_GamePlay/UI/UI.h"
#include "Source/04_GamePlay/GameObjectManager.h"
#include "Source/04_GamePlay/CameraDirector.h"
#include "Source/04_GamePlay/GameEnvironment.h"
#include <memory>
#include <vector>

class GameScene : public Scene {
public:
    /**
     * @brief ゲームシーンのコンストラクタ
     */
    GameScene();

    /**
     * @brief ゲームシーンのデストラクタ
     */
    ~GameScene();

    static std::unique_ptr<GameScene> s_transferInstance;

    /**
     * @brief シーンの初期化
     */
    bool Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine) override;

    bool InitializeEnvironment(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine);
    bool InitializeUI();
    bool InitializeGameObjects();
    bool InitializeAudio();

    /**
     * @brief シーンの終了処理
     */
    void Shutdown() override;

    /**
     * @brief シーンの更新処理
     * @param deltaTime 前フレームからの経過時間
     */
    void Update(float deltaTime) override;

    /**
     * @brief シーンの描画処理
     */
    void Render() override;

    void UpdateTitleLoop(float deltaTime);
    void RenderStageOnly();
    void SetCameraForTitle();
    void BeginOpening();

private:
    bool m_isPaused = false;
    int m_pauseSelectIndex = 0;

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