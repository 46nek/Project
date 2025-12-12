#pragma once
#include "Scene.h"
#include "Sprite.h"
#include <memory>
#include "SpriteBatch.h"

/**
 * @brief リザルト画面のシーンを管理するクラス
 */
class ResultScene : public Scene {
public:
	ResultScene();
	~ResultScene();

	bool Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine) override;
	void Shutdown() override;
	void Update(float deltaTime) override;
	void Render() override;

private:
	std::unique_ptr<Sprite> m_background;
	std::unique_ptr<Sprite> m_pressEnter;   
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

	float m_clearTextScale;
	float m_pressEnterScale;
};