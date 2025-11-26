#pragma once
#include "Scene.h"
#include "GameScene.h"
#include <memory>
#include "SpriteBatch.h"
#include "FW1FontWrapper.h"

class LoadingScene : public Scene
{
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