#include "Input.h"
#include <algorithm> 
#include <iterator>

Input::Input() : m_mouseX(0), m_mouseY(0)
{
}

Input::~Input()
{
}

void Input::Initialize()
{
	std::fill(std::begin(m_keys), std::end(m_keys), false);
	std::fill(std::begin(m_previousKeys), std::end(m_previousKeys), false);

	// マウス移動量を0で初期化
	m_mouseX = 0;
	m_mouseY = 0;
}

void Input::EndFrame()
{
	for (int i = 0; i < KEY_COUNT; i++)
	{
		m_previousKeys[i] = m_keys[i];
	}

	// 次のフレームのためにマウス移動量をリセット
	m_mouseX = 0;
	m_mouseY = 0;
}

void Input::KeyDown(unsigned int input)
{
	// キーが押されたことを記録
	m_keys[input] = true;
}

void Input::KeyUp(unsigned int input)
{
	// キーが離されたことを記録
	m_keys[input] = false;
}

bool Input::IsKeyDown(unsigned int key)
{
	// 指定されたキーが押されているかどうかの状態を返す
	return m_keys[key];
}

bool Input::IsKeyPressed(unsigned int key)
{
	// 現在は押されていて、前のフレームでは押されていなかった場合にtrueを返す
	return m_keys[key] && !m_previousKeys[key];
}
void Input::MouseMove(int x, int y)
{
	// 前のフレームからの移動量を加算
	m_mouseX += x;
	m_mouseY += y;
}

void Input::GetMouseDelta(int& x, int& y)
{
	x = m_mouseX;
	y = m_mouseY;
}