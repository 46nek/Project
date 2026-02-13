#pragma once
#include "Scene.h"
#include "Camera.h"
#include "Player.h"
#include "UI.h"
#include "GameObjectManager.h"
#include "CameraDirector.h"
#include "GameEnvironment.h"
#include <memory>
#include <vector>

class GameScene : public Scene {
public:
    /**
     * @brief 繧ｲ繝ｼ繝繧ｷ繝ｼ繝ｳ縺ｮ繧ｳ繝ｳ繧ｹ繝医Λ繧ｯ繧ｿ
     */
    GameScene();

    /**
     * @brief 繧ｲ繝ｼ繝繧ｷ繝ｼ繝ｳ縺ｮ繝・せ繝医Λ繧ｯ繧ｿ
     */
    ~GameScene();

    static std::unique_ptr<GameScene> s_transferInstance;

    /**
     * @brief 繧ｷ繝ｼ繝ｳ縺ｮ蛻晄悄蛹・
     */
    bool Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine) override;

    bool InitializeEnvironment(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine);
    bool InitializeUI();
    bool InitializeGameObjects();
    bool InitializeAudio();

    /**
     * @brief 繧ｷ繝ｼ繝ｳ縺ｮ邨ゆｺ・・逅・
     */
    void Shutdown() override;

    /**
     * @brief 繧ｷ繝ｼ繝ｳ縺ｮ譖ｴ譁ｰ蜃ｦ逅・
     * @param deltaTime 蜑阪ヵ繝ｬ繝ｼ繝縺九ｉ縺ｮ邨碁℃譎る俣
     */
    void Update(float deltaTime) override;

    /**
     * @brief 繧ｷ繝ｼ繝ｳ縺ｮ謠冗判蜃ｦ逅・
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
