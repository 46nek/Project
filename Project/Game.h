#pragma once
#include "Window.h"
#include "GraphicsDevice.h"
#include "Input.h"
#include "SceneManager.h"
#include "Timer.h"
#include "Audio.h"
#include "GameSettings.h"
#include <memory>

class SceneManager;

/**
 * @brief 繧ｲ繝ｼ繝蜈ｨ菴薙・邂｡逅・ｒ陦後≧繝｡繧､繝ｳ繧ｯ繝ｩ繧ｹ
 */
class Game {
public:
	// 螳壽焚
	static constexpr int SCREEN_WIDTH = 1280;
	static constexpr int SCREEN_HEIGHT = 720;

	Game();
	~Game();

	/**
	 * @brief 繧ｲ繝ｼ繝縺ｮ蛻晄悄蛹門・逅・
	 * @param hInstance 繧｢繝励Μ繧ｱ繝ｼ繧ｷ繝ｧ繝ｳ縺ｮ繧､繝ｳ繧ｹ繧ｿ繝ｳ繧ｹ繝上Φ繝峨Ν
	 * @return 蛻晄悄蛹悶′謌仙粥縺励◆蝣ｴ蜷医・ true
	 */
	bool Initialize(HINSTANCE hInstance);

	/**
	 * @brief 繧ｲ繝ｼ繝縺ｮ繝｡繧､繝ｳ繝ｫ繝ｼ繝励ｒ螳溯｡・
	 */
	void Run();

	/**
	 * @brief 繧ｲ繝ｼ繝縺ｮ邨ゆｺ・・逅・
	 */
	void Shutdown();

	/**
	 * @brief 繧ｲ繝ｼ繝縺ｮ繝昴・繧ｺ迥ｶ諷九ｒ險ｭ螳・
	 * @param isPaused 繝昴・繧ｺ迥ｶ諷九↓縺吶ｋ蝣ｴ蜷医・ true
	 */
	void SetPaused(bool isPaused);

	/**
	 * @brief 迴ｾ蝨ｨ縺ｮ繝昴・繧ｺ迥ｶ諷九ｒ蜿門ｾ・
	 * @return 繝昴・繧ｺ荳ｭ縺ｮ蝣ｴ蜷医・ true
	 */
	bool IsPaused() const;

	Window* GetWindow() const { return m_window.get(); }
	GameSettings& GetSettings() { return m_settings; }
	DirectX::AudioEngine* GetAudioEngine() const { return m_audioEngine.get(); }
private:
	/**
	 * @brief 繝輔Ξ繝ｼ繝縺斐→縺ｮ譖ｴ譁ｰ蜃ｦ逅・
	 * @return 繧ｲ繝ｼ繝縺ｮ螳溯｡後ｒ邯壹￠繧句ｴ蜷医・ true
	 */
	bool Update();

	/**
	 * @brief 繝輔Ξ繝ｼ繝縺斐→縺ｮ謠冗判蜃ｦ逅・
	 */
	void Render();

	// 繝｡繝ｳ繝舌・螟画焚
	std::unique_ptr<Window> m_window;
	std::unique_ptr<Input> m_input;
	std::unique_ptr<GraphicsDevice> m_graphicsDevice;
	std::unique_ptr<SceneManager> m_sceneManager;
	std::unique_ptr<Timer> m_timer;
	std::unique_ptr<DirectX::AudioEngine> m_audioEngine;

	GameSettings m_settings;
	bool m_isPaused;
};
