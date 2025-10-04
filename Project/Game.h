// Game.h
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Direct3D.h"
#include "Camera.h"
#include "Input.h"

class Game
{
public:
    Game();
    ~Game();

    bool Initialize();
    void Run();
    void Shutdown();

    LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
    bool Frame();
    void InitializeWindows(int&, int&);
    void ShutdownWindows();

private:
    LPCWSTR m_applicationName;
    HINSTANCE m_hinstance;
    HWND m_hwnd;

    Direct3D* m_D3D; // Direct3D�N���X�̃C���X�^���X
    Camera* m_Camera;
    Input* m_Input;

    int m_screenWidth, m_screenHeight;
};

// Windows�v���V�[�W���̂��߂̐ÓI�|�C���^
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static Game* ApplicationHandle = 0;