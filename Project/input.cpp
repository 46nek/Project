#include "Input.h"

Input::Input()
{
}

Input::~Input()
{
}

void Input::Initialize()
{
    // すべてのキーの状態を '押されていない' (false) で初期化
    for (int i = 0; i < 256; i++)
    {
        m_keys[i] = false;
    }
    // マウス移動量を0で初期化
    m_mouseX = 0;
    m_mouseY = 0;
}

void Input::Frame()
{
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