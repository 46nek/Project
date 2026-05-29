#pragma once
#include "Source/01_Scene/Scene.h"
#include "Source/01_Scene/GameScene.h"
#include "SpriteBatch.h"
#include "FW1FontWrapper.h"
#include <memory>

class LoadingScene : public Scene {
public:
	LoadingScene();
	~LoadingScene();

	bool Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine) override;
	void Shutdown() override;
	void Update(float deltaTime) override;
	void Render() override;

	std::unique_ptr<GameScene> GetGameScene() { return std::move(m_gameScene); }
private:
	std::unique_ptr<GameScene> m_gameScene;
	int m_loadingPhase;

	IFW1Factory* m_fontFactory;
	IFW1FontWrapper* m_fontWrapper;
};
