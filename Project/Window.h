#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Input.h"

/**
 * @brief �E�B���h�E�̐����ƊǗ����s���N���X
 */
class Window
{
public:
    Window(LPCWSTR applicationName, int screenWidth, int screenHeight);
    ~Window();

    /**
     * @brief �E�B���h�E��������
     * @param hInstance �A�v���P�[�V�����̃C���X�^���X�n���h��
     * @param input ���͊Ǘ��N���X�̃C���X�^���X
     * @return �����������������ꍇ�� true
     */
    bool Initialize(HINSTANCE hInstance, Input* input);

    /**
     * @brief �E�B���h�E��j��
     */
    void Shutdown();

    /**
     * @brief �E�B���h�E�n���h�����擾
     * @return HWND �E�B���h�E�n���h��
     */
    HWND GetHwnd() const;

private:
    /**
     * @brief �E�B���h�E�v���V�[�W��
     */
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

    /**
     * @brief ���b�Z�[�W�n���h��
     */
    LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

    LPCWSTR m_applicationName;
    HINSTANCE m_hinstance;
    HWND m_hwnd;
    Input* m_input;
    int m_screenWidth;
    int m_screenHeight;
};