#include "Window.h"

// WndProc�Ŏg�����߂̐ÓI�|�C���^
static Window* g_windowHandle = nullptr;

Window::Window(LPCWSTR applicationName, int screenWidth, int screenHeight)
    : m_applicationName(applicationName),
    m_screenWidth(screenWidth),
    m_screenHeight(screenHeight),
    m_hinstance(nullptr),
    m_hwnd(nullptr),
    m_input(nullptr)
{
}

Window::~Window()
{
}

bool Window::Initialize(HINSTANCE hInstance, Input* input)
{
    g_windowHandle = this;
    m_hinstance = hInstance;
    m_input = input;

    WNDCLASSEX wc = {};
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_hinstance;
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = m_applicationName;
    wc.cbSize = sizeof(WNDCLASSEX);

    if (!RegisterClassEx(&wc))
    {
        return false;
    }

    m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, m_screenWidth, m_screenHeight, NULL, NULL, m_hinstance, NULL);

    if (!m_hwnd)
    {
        return false;
    }

    RAWINPUTDEVICE rid = {};
    rid.usUsagePage = 0x01; // Generic Desktop
    rid.usUsage = 0x02;     // Mouse
    rid.dwFlags = RIDEV_NOLEGACY;
    rid.hwndTarget = m_hwnd;
    RegisterRawInputDevices(&rid, 1, sizeof(rid));

    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
    SetFocus(m_hwnd);
    ShowCursor(false);

    return true;
}

void Window::Shutdown()
{
    ShowCursor(true);
    if (m_hwnd)
    {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
    if (m_hinstance)
    {
        UnregisterClass(m_applicationName, m_hinstance);
        m_hinstance = nullptr;
    }
    g_windowHandle = nullptr;
}

HWND Window::GetHwnd() const
{
    return m_hwnd;
}

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
    switch (umessage)
    {
    case WM_DESTROY:
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    default:
        if (g_windowHandle)
        {
            return g_windowHandle->MessageHandler(hwnd, umessage, wparam, lparam);
        }
        else
        {
            return DefWindowProc(hwnd, umessage, wparam, lparam);
        }
    }
}

LRESULT CALLBACK Window::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
    switch (umsg)
    {
    case WM_INPUT:
    {
        UINT dwSize = sizeof(RAWINPUT);
        static BYTE lpb[sizeof(RAWINPUT)];
        GetRawInputData((HRAWINPUT)lparam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));
        RAWINPUT* raw = (RAWINPUT*)lpb;

        if (raw->header.dwType == RIM_TYPEMOUSE)
        {
            m_input->MouseMove(raw->data.mouse.lLastX, raw->data.mouse.lLastY);
        }
        return 0;
    }
    case WM_KEYDOWN:
        m_input->KeyDown((unsigned int)wparam);
        return 0;
    case WM_KEYUP:
        m_input->KeyUp((unsigned int)wparam);
        return 0;
    case WM_SETCURSOR:
        ShowCursor(false);
        return true;
    case WM_KILLFOCUS:
        m_input->Initialize();
        return 0;
    default:
        return DefWindowProc(hwnd, umsg, wparam, lparam);
    }
}