#pragma once
#include "Scene.h"
#include <memory>
#include <vector>
#include "Camera.h"
#include "Player.h"
#include "LightManager.h"
#include "Renderer.h"
#include "Stage.h"
#include "Enemy.h"
#include "Orb.h"
#include "UI.h"

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

	void UpdateTitleLoop(float deltaTime);

	void RenderStageOnly();
	void SetCameraForTitle();
	void BeginOpening();

private:
	bool InitializeEnemies();
	bool InitializeOrbs();
	bool InitializeSpecialOrbs();

	void UpdateVignette(float staminaPercentage);

	void UpdateOpening(float deltaTime);

	static constexpr float PLAYER_HEIGHT = 3.0f;
	static constexpr int NUM_ENEMIES = 2;
	static constexpr int NUM_ORBS = 20;

	// UIフェードインにかける時間（秒）
	static constexpr float UI_FADE_DURATION = 0.5f;

	std::unique_ptr<DirectX::SoundEffect> m_collectSound;
	std::unique_ptr<DirectX::SoundEffect> m_walkSoundEffect;
	std::unique_ptr<DirectX::SoundEffect> m_runSoundEffect;

	std::unique_ptr<Stage> m_stage;
	std::unique_ptr<Camera> m_camera;
	std::unique_ptr<LightManager> m_lightManager;
	std::unique_ptr<Renderer> m_renderer;

	std::unique_ptr<Player> m_player;
	std::vector<std::unique_ptr<Enemy>> m_enemies;
	std::vector<std::unique_ptr<Orb>> m_orbs;
	std::vector<std::unique_ptr<Orb>> m_specialOrbs; // 特殊オーブのリスト
	float m_enemyRadarTimer; // 敵レーダーの効果時間タイマー
	std::unique_ptr<UI> m_ui;

	std::vector<Model*> m_cachedStageModels;
	std::vector<Model*> m_cachedDynamicModels;

	float m_vignetteIntensity;

	int m_remainingOrbs;
	int m_totalOrbs;

	std::unique_ptr<Orb> m_goalOrb;
	bool m_goalSpawned;
	bool m_escapeMode;

	bool m_isOpening;           // オープニング演出中か
	float m_openingTimer;       // 演出経過時間
	float m_openingDuration;    // 演出にかける時間
	DirectX::XMFLOAT3 m_titleCamPos; // タイトル画面のカメラ位置
	DirectX::XMFLOAT3 m_titleCamRot; // タイトル画面のカメラ回転
	DirectX::XMFLOAT3 m_startCamPos; // ゲーム開始時のカメラ位置
	DirectX::XMFLOAT3 m_startCamRot; // ゲーム開始時のカメラ回転

	float m_titleTimer;
	float m_uiFadeTimer;        // UIフェードイン用のタイマー
};