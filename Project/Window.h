#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "input.h"

class Window
{
public:
    Window(LPCWSTR applicationName, int screenWidth, int screenHeight);
    ~Window();

    bool Initialize(HINSTANCE hInstance, Input* input);
    void Shutdown();

    HWND GetHwnd() const;

private:
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);


    LPCWSTR m_applicationName;
    HINSTANCE m_hinstance;
    HWND m_hwnd;
    Input* m_input;
    int m_screenWidth, m_screenHeight;
};