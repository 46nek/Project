#pragma once
#include "Scene.h"
#include "Sprite.h"
#include "SpriteBatch.h"
#include "GameScene.h"
#include "FW1FontWrapper.h"
#include <memory> 

/**
 * @brief タイトル画面のシーンを管理するクラス
 */
class TitleScene : public Scene {
public:
	TitleScene();
	~TitleScene();

	bool Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine) override;
	void Shutdown() override;
	void Update(float deltaTime) override;
	void Render() override;

private:
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

	std::unique_ptr<GameScene> m_gameScene;

	//フォント関連
	IFW1Factory* m_fontFactory;
	std::vector<IFW1FontWrapper*> m_fonts;

	// 各文字の状態を管理する構造体
	struct CharState {
		int fontIndex; 
	};

	// タイトルテキスト
	std::wstring m_titleText;
	std::vector<CharState> m_charStates;

	// PLAYボタンテキスト
	std::wstring m_playText;
	std::vector<CharState> m_playCharStates;
	bool m_isPlayHovered;

	// グリッチ更新用タイマー
	float m_glitchTimer;
	float m_glitchUpdateInterval;

	std::wstring m_settingText;
	std::vector<CharState> m_settingCharStates;
	bool m_isSettingHovered;
};