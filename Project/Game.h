#pragma once
#include <memory>
#include "Window.h"
#include "GraphicsDevice.h"
#include "Input.h"
#include "SceneManager.h"
#include "Timer.h"

/**
 * @brief ゲーム全体の管理を行うメインクラス
 */
class Game
{
public:
	// 定数
	static constexpr int SCREEN_WIDTH = 1280;
	static constexpr int SCREEN_HEIGHT = 720;

	Game();
	~Game();

	/**
	 * @brief ゲームの初期化処理
	 * @param hInstance アプリケーションのインスタンスハンドル
	 * @return 初期化が成功した場合は true
	 */
	bool Initialize(HINSTANCE hInstance);

	/**
	 * @brief ゲームのメインループを実行
	 */
	void Run();

	/**
	 * @brief ゲームの終了処理
	 */
	void Shutdown();

	/**
	 * @brief ゲームのポーズ状態を設定
	 * @param isPaused ポーズ状態にする場合は true
	 */
	void SetPaused(bool isPaused);

	/**
	 * @brief 現在のポーズ状態を取得
	 * @return ポーズ中の場合は true
	 */
	bool IsPaused() const;

private:
	/**
	 * @brief フレームごとの更新処理
	 * @return ゲームの実行を続ける場合は true
	 */
	bool Update();

	/**
	 * @brief フレームごとの描画処理
	 */
	void Render();

	// メンバー変数
	std::unique_ptr<Window> m_window;
	std::unique_ptr<Input> m_input;
	std::unique_ptr<GraphicsDevice> m_graphicsDevice;
	std::unique_ptr<SceneManager> m_sceneManager;
	std::unique_ptr<Timer> m_timer;
	bool m_isPaused;
};