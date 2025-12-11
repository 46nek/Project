#pragma once
#include "Scene.h"
#include <memory>
#include <vector>
#include "Camera.h"
#include "Player.h"
#include "LightManager.h"
#include "Renderer.h"
#include "Stage.h"
#include "UI.h"
#include "GameObjectManager.h"
#include "CameraDirector.h" 

class GameScene : public Scene
{
public:
	GameScene();
	~GameScene();

	static std::unique_ptr<GameScene> s_transferInstance;

	bool Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine) override;
	bool InitializePhase1(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine);
	bool InitializePhase2();
	bool InitializePhase3();
	bool InitializePhase4();
	bool InitializePhase5();
	void Shutdown() override;
	void Update(float deltaTime) override;
	void Render() override;

	// TitleSceneから呼ばれる関数（中身はDirectorに丸投げします）
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

	std::unique_ptr<Stage> m_stage;
	std::shared_ptr<Camera> m_camera; // ★ unique_ptr から shared_ptr に変更（Directorと共有するため）
	std::unique_ptr<LightManager> m_lightManager;
	std::unique_ptr<Renderer> m_renderer;

	std::unique_ptr<Player> m_player;
	std::unique_ptr<UI> m_ui;

	// マネージャークラス
	std::unique_ptr<GameObjectManager> m_gameObjectManager;
	std::unique_ptr<CameraDirector> m_cameraDirector; // ★ 追加

	std::vector<Model*> m_cachedStageModels;
	std::vector<Model*> m_cachedDynamicModels;

	float m_uiFadeTimer;
};