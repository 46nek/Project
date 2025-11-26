#pragma once

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
	bool m_keys[KEY_COUNT];
	bool m_previousKeys[KEY_COUNT];
	int m_mouseX, m_mouseY;
};