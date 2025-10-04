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