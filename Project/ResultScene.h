#pragma once
#include "Scene.h"
#include "Sprite.h"
#include "SpriteBatch.h"
#include "FW1FontWrapper.h"
#include <memory>
#include <vector>
#include <string>

/**
 * @brief 繝ｪ繧ｶ繝ｫ繝育判髱｢縺ｮ繧ｷ繝ｼ繝ｳ繧堤ｮ｡逅・☆繧九け繝ｩ繧ｹ
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

	// --- 繝輔か繝ｳ繝医・繧ｰ繝ｪ繝・メ髢｢騾｣ ---
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

	// --- 繝槭え繧ｹ驕ｸ謚樣未騾｣ ---
	bool m_isTitleHovered;
	bool m_isExitHovered;

	float m_glitchTimer;
	float m_glitchUpdateInterval;

	// 謠冗判繝ｻ險育ｮ苓｣懷勧髢｢謨ｰ
	void DrawGlitchText(const std::wstring& text, std::vector<CharState>& states, float fontSize, float startY, bool isHovered);
	float CalculateTextWidth(const std::wstring& text, float fontSize);
};
