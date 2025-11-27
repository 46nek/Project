#pragma once
#include "Scene.h"
#include "Sprite.h"
#include <memory>
#include "SpriteBatch.h"
#include "GameScene.h"
/**
 * @brief タイトル画面のシーンを管理するクラス
 */
class TitleScene : public Scene
{
public:
	TitleScene();
	~TitleScene();

	bool Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine) override;
	void Shutdown() override;
	void Update(float deltaTime) override;
	void Render() override;

private:
	std::unique_ptr<Sprite> m_titleLogo;
	std::unique_ptr<Sprite> m_pressEnter;
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

	float m_titleLogoScale;
	float m_pressEnterScale;

	std::unique_ptr<GameScene> m_gameScene;
};