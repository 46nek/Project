#pragma once

/**
 * @brief キーボードとマウスの入力を管理するクラス
 */
class Input
{
public:
	Input();
	~Input();

	void Initialize();
	/**
	 * @brief フレームの終わりにキー状態を更新
	 */
	void EndFrame();
	void KeyDown(unsigned int);
	void KeyUp(unsigned int);
	void MouseMove(int x, int y);

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

private:
	bool m_keys[256];
	bool m_previousKeys[256];
	int m_mouseX, m_mouseY;
};