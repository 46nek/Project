#pragma once
#include "Scene.h"
#include "Sprite.h"
#include <memory>
#include <vector>
#include <string>
#include "SpriteBatch.h"
#include "FW1FontWrapper.h"

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
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

	// --- フォント・グリッチ関連 ---
	IFW1Factory* m_fontFactory;
	std::vector<IFW1FontWrapper*> m_fonts;

	struct CharState {
		int fontIndex;
	};

	std::wstring m_escapeText;
	std::vector<CharState> m_escapeCharStates;

	std::wstring m_toTitleText;
	std::vector<CharState> m_toTitleCharStates;

	std::wstring m_exitText;
	std::vector<CharState> m_exitCharStates;

	// --- マウス選択関連 ---
	bool m_isTitleHovered;
	bool m_isExitHovered;

	float m_glitchTimer;
	float m_glitchUpdateInterval;

	// 描画・計算補助関数
	void DrawGlitchText(const std::wstring& text, std::vector<CharState>& states, float fontSize, float startY, bool isHovered);
	float CalculateTextWidth(const std::wstring& text, float fontSize);
};