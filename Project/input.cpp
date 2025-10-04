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
    // �}�E�X�ړ��ʂ�0�ŏ�����
    m_mouseX = 0;
    m_mouseY = 0;
}

void Input::Frame()
{
    // ���̃t���[���̂��߂Ƀ}�E�X�ړ��ʂ����Z�b�g
    m_mouseX = 0;
    m_mouseY = 0;
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

void Input::MouseMove(int x, int y)
{
    // �O�̃t���[������̈ړ��ʂ����Z
    m_mouseX += x;
    m_mouseY += y;
}

void Input::GetMouseDelta(int& x, int& y)
{
    x = m_mouseX;
    y = m_mouseY;
}