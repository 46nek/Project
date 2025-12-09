#pragma once
#include "Scene.h"
#include "Sprite.h"
#include <memory>
#include "SpriteBatch.h"
#include "GameScene.h"
#include "FW1FontWrapper.h"

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
	std::unique_ptr<Sprite> m_pressEnter;
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

	float m_pressEnterScale;

	std::unique_ptr<GameScene> m_gameScene;

	// --- フォント関連 ---
	IFW1Factory* m_fontFactory;
	// 0番目をメイン、それ以降をグリッチ用とする複数のフォントラッパー
	std::vector<IFW1FontWrapper*> m_fonts;

	// タイトルテキスト
	std::wstring m_titleText;

	// 各文字の状態を管理する構造体
	struct CharState {
		int fontIndex; // 現在適用されているフォントのインデックス
	};
	std::vector<CharState> m_charStates;

	// グリッチ更新用タイマー
	float m_glitchTimer;
	float m_glitchUpdateInterval;
};