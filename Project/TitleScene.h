#pragma once
#include "Scene.h"
#include "Sprite.h"
#include "SpriteBatch.h"
#include "GameScene.h"
#include "FW1FontWrapper.h"
#include <memory> 

/**
 * @brief 繧ｿ繧､繝医Ν逕ｻ髱｢縺ｮ繧ｷ繝ｼ繝ｳ繧堤ｮ｡逅・☆繧九け繝ｩ繧ｹ
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

	// --- 繝輔か繝ｳ繝磯未騾｣ ---
	IFW1Factory* m_fontFactory;
	// 0逡ｪ逶ｮ繧偵Γ繧､繝ｳ縲√◎繧御ｻ･髯阪ｒ繧ｰ繝ｪ繝・メ逕ｨ縺ｨ縺吶ｋ隍・焚縺ｮ繝輔か繝ｳ繝医Λ繝・ヱ繝ｼ
	std::vector<IFW1FontWrapper*> m_fonts;

	// 蜷・枚蟄励・迥ｶ諷九ｒ邂｡逅・☆繧区ｧ矩菴・
	struct CharState {
		int fontIndex; // 迴ｾ蝨ｨ驕ｩ逕ｨ縺輔ｌ縺ｦ縺・ｋ繝輔か繝ｳ繝医・繧､繝ｳ繝・ャ繧ｯ繧ｹ
	};

	// 繧ｿ繧､繝医Ν繝・く繧ｹ繝・
	std::wstring m_titleText;
	std::vector<CharState> m_charStates;

	// PLAY繝懊ち繝ｳ繝・く繧ｹ繝・
	std::wstring m_playText;
	std::vector<CharState> m_playCharStates;
	bool m_isPlayHovered; // 繧ｫ繝ｼ繧ｽ繝ｫ縺悟粋縺｣縺ｦ縺・ｋ縺・

	// 繧ｰ繝ｪ繝・メ譖ｴ譁ｰ逕ｨ繧ｿ繧､繝槭・
	float m_glitchTimer;
	float m_glitchUpdateInterval;

	std::wstring m_settingText;
	std::vector<CharState> m_settingCharStates;
	bool m_isSettingHovered;
};
