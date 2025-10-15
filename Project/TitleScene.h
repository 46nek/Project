#pragma once
#include "Scene.h"
#include "Sprite.h"
#include <memory>
#include "SpriteBatch.h"

/**
 * @brief タイトル画面のシーンを管理するクラス
 */
class TitleScene : public Scene
{
public:
	TitleScene();
	~TitleScene();

	bool Initialize(GraphicsDevice* graphicsDevice, Input* input) override;
	void Shutdown() override;
	void Update(float deltaTime) override;
	void Render() override;

private:
	std::unique_ptr<Sprite> m_background;
	std::unique_ptr<Sprite> m_titleLogo;
	std::unique_ptr<Sprite> m_pressEnter;
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

	// ▼▼▼ 以下を追加 ▼▼▼
	float m_titleLogoScale;
	float m_pressEnterScale;
	// ▲▲▲ 追加ここまで ▲▲▲
};