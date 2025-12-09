#pragma once
#include <Windows.h> // HWNDを使用するために追加

/**
 * @brief キーボードとマウスの入力を管理するクラス
 */
class Input
{
public:
	static constexpr int KEY_COUNT = 256;

	Input();
	~Input();

	void Initialize();
	/**
	 * @brief フレームの終わりにキー状態を更新
	 */
	void EndFrame();
	/**
	 * @brief 毎フレームの更新処理（カーソル固定などを行う）
	 * @param hwnd ウィンドウハンドル（カーソル固定の基準位置計算用）
	 */
	void Update(HWND hwnd);

	void KeyDown(unsigned int);
	void KeyUp(unsigned int);
	void MouseMove(int x, int y);

	/**
	 * @brief マウスの絶対座標を設定
	*/
	void SetMousePosition(int x, int y);

	/**
	 * @brief マウスの絶対座標を取得
	 */
	void GetMousePosition(int& x, int& y);

	/**
	 * @brief 指定キーが現在押されているか
	 */
	bool IsKeyDown(unsigned int key);
	/**
	 * @brief 指定キーがこのフレームで押された瞬間か
	 */
	bool IsKeyPressed(unsigned int key);
	/**
	 * @brief 前フレームからのマウスの移動量を取得
	 */
	void GetMouseDelta(int& x, int& y);

	/**
	 * @brief カーソルの表示/非表示を設定
	 */
	void SetCursorVisible(bool visible);

	/**
	 * @brief カーソルのロック（中央固定）状態を設定
	 */
	void SetCursorLock(bool lock);

	/**
	 * @brief カーソルがロックされているかを取得
	 */
	bool IsCursorLocked() const;

private:
	bool m_keys[KEY_COUNT];
	bool m_previousKeys[KEY_COUNT];
	int m_mouseX, m_mouseY;
	int m_absMouseX, m_absMouseY;

	// カーソル制御用
	bool m_isCursorLocked;   // カーソルを固定するかどうか
	bool m_isCursorVisible;  // 現在のカーソル表示状態管理（ShowCursorの重複呼び出し防止用）
};