#include "Input.h"

Input::Input()
{
}

Input::~Input()
{
}

void Input::Initialize()
{
    // ���ׂẴL�[�̏�Ԃ� '������Ă��Ȃ�' (false) �ŏ�����
    for (int i = 0; i < 256; i++)
    {
        m_keys[i] = false;
    }
}

void Input::KeyDown(unsigned int input)
{
    // �L�[�������ꂽ���Ƃ��L�^
    m_keys[input] = true;
}

void Input::KeyUp(unsigned int input)
{
    // �L�[�������ꂽ���Ƃ��L�^
    m_keys[input] = false;
}

bool Input::IsKeyDown(unsigned int key)
{
    // �w�肳�ꂽ�L�[��������Ă��邩�ǂ����̏�Ԃ�Ԃ�
    return m_keys[key];
}