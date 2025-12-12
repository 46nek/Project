#include "Input.h"
#include <algorithm> 
#include <iterator>

Input::Input() :
	m_mouseX(0), m_mouseY(0), m_absMouseX(0), m_absMouseY(0),
	m_isCursorLocked(false), m_isCursorVisible(true) // 初期化
{
}

Input::~Input() {
}

void Input::Initialize() {
	std::fill(std::begin(m_keys), std::end(m_keys), false);
	std::fill(std::begin(m_previousKeys), std::end(m_previousKeys), false);

	// マウス移動量を0で初期化
	m_mouseX = 0;
	m_mouseY = 0;
	m_absMouseX = 0;
	m_absMouseY = 0;
}

void Input::EndFrame() {
	for (int i = 0; i < KEY_COUNT; i++) {
		m_previousKeys[i] = m_keys[i];
	}

	// 次のフレームのためにマウス移動量をリセット
	m_mouseX = 0;
	m_mouseY = 0;
}

void Input::Update(HWND hwnd) {
	// カーソルロックが有効なら、カーソルを画面中央に戻す
	if (m_isCursorLocked && hwnd) {
		// ウィンドウがアクティブな場合のみ実行（非アクティブ時にマウスを奪わないように）
		if (GetForegroundWindow() == hwnd) {
			// ウィンドウの中央座標を計算
			RECT rect;
			GetClientRect(hwnd, &rect);
			POINT center;
			center.x = (rect.right - rect.left) / 2;
			center.y = (rect.bottom - rect.top) / 2;

			// スクリーン座標に変換してセット
			ClientToScreen(hwnd, &center);
			SetCursorPos(center.x, center.y);
		}
	}
}

void Input::KeyDown(unsigned int input) {
	// キーが押されたことを記録
	m_keys[input] = true;
}

void Input::KeyUp(unsigned int input) {
	// キーが離されたことを記録
	m_keys[input] = false;
}

void Input::MouseMove(int x, int y) {
	// 前のフレームからの移動量を加算
	m_mouseX += x;
	m_mouseY += y;
}

void Input::SetMousePosition(int x, int y) {
	m_absMouseX = x;
	m_absMouseY = y;
}

void Input::GetMousePosition(int& x, int& y) {
	x = m_absMouseX;
	y = m_absMouseY;
}

bool Input::IsKeyDown(unsigned int key) {
	// 指定されたキーが押されているかどうかの状態を返す
	return m_keys[key];
}

bool Input::IsKeyPressed(unsigned int key) {
	// 現在は押されていて、前のフレームでは押されていなかった場合にtrueを返す
	return m_keys[key] && !m_previousKeys[key];
}

void Input::GetMouseDelta(int& x, int& y) {
	x = m_mouseX;
	y = m_mouseY;
}

void Input::SetCursorVisible(bool visible) {
	// 状態が変わる場合のみAPIを呼ぶ（ShowCursorは内部カウンタを持つため）
	if (m_isCursorVisible != visible) {
		::ShowCursor(visible ? TRUE : FALSE);
		m_isCursorVisible = visible;
	}
}

void Input::SetCursorLock(bool lock) {
	m_isCursorLocked = lock;

	// ロックするときはカーソルを消すのが一般的
	if (lock) {
		SetCursorVisible(false);
	}
	else {
		SetCursorVisible(true);
	}
}

bool Input::IsCursorLocked() const {
	return m_isCursorLocked;
}