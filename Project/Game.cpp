#include "Game.h"

Game::Game()
{
    m_applicationName = nullptr;
    m_hinstance = nullptr;
    m_hwnd = nullptr;
    m_screenWidth = 0;
    m_screenHeight = 0;

    m_D3D = nullptr;
    m_Input = nullptr;
    m_SceneManager = nullptr; // �ǉ�
    ApplicationHandle = this;
}

Game::~Game()
{
}

bool Game::Initialize()
{
    // �E�B���h�E�T�C�Y��������
    m_screenWidth = 0;
    m_screenHeight = 0;

    // Windows API��������
    InitializeWindows(m_screenWidth, m_screenHeight);

    // Input�I�u�W�F�N�g���쐬���ď�����
    m_Input = new Input;
    if (!m_Input) return false;
    m_Input->Initialize();

    // Direct3D�I�u�W�F�N�g���쐬
    m_D3D = new Direct3D;
    if (!m_D3D) return false;

    // Direct3D��������
    if (!m_D3D->Initialize(m_hwnd, m_screenWidth, m_screenHeight))
    {
        MessageBox(m_hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
        return false;
    }

    // SceneManager�I�u�W�F�N�g���쐬���ď�����
    m_SceneManager = new SceneManager;
    if (!m_SceneManager) return false;
    if (!m_SceneManager->Initialize(m_D3D, m_Input)) return false;


    return true;
}

void Game::Shutdown()
{
    // SceneManager�I�u�W�F�N�g�����
    if (m_SceneManager)
    {
        m_SceneManager->Shutdown();
        delete m_SceneManager;
        m_SceneManager = nullptr;
    }

    if (m_Input)
    {
        delete m_Input;
        m_Input = nullptr;
    }

    // Direct3D�I�u�W�F�N�g�����
    if (m_D3D)
    {
        m_D3D->Shutdown();
        delete m_D3D;
        m_D3D = nullptr;
    }

    // �E�B���h�E�����
    ShutdownWindows();
}

void Game::Run()
{
    MSG msg;
    bool result;

    ZeroMemory(&msg, sizeof(MSG));

    // ���C�����[�v
    while (true)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            result = Frame();
            if (!result)
            {
                break;
            }
        }
    }
}

bool Game::Frame()
{
    // ���t���[���̏�����SceneManager�ɈϏ�
    m_SceneManager->Update(0.0f); // deltaTime�͊e�V�[���̃^�C�}�[�ŊǗ�
    m_SceneManager->Render();

    // ���̃t���[���̂��߂ɁA�t���[���̍Ō�Ƀ}�E�X�ړ��ʂ����Z�b�g
    m_Input->Frame();

    return true;
}

// MessageHandler, InitializeWindows, ShutdownWindows, WndProc �͕ύX�Ȃ�
// (�������AGame::Frame()���̏I���m�F���W�b�N��GameScene�Ɉړ������̂ō폜)
LRESULT CALLBACK Game::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
    switch (umsg)
    {
        // �����x�}�E�X���͂̃��b�Z�[�W
    case WM_INPUT:
    {
        UINT dwSize = sizeof(RAWINPUT);
        static BYTE lpb[sizeof(RAWINPUT)];

        GetRawInputData((HRAWINPUT)lparam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

        RAWINPUT* raw = (RAWINPUT*)lpb;

        if (raw->header.dwType == RIM_TYPEMOUSE)
        {
            m_Input->MouseMove(raw->data.mouse.lLastX, raw->data.mouse.lLastY);
        }
        return 0;
    }
    // �L�[�������ꂽ���b�Z�[�W
    case WM_KEYDOWN:
    {
        m_Input->KeyDown((unsigned int)wparam);
        return 0;
    }

    // �L�[�������ꂽ���b�Z�[�W
    case WM_KEYUP:
    {
        m_Input->KeyUp((unsigned int)wparam);
        return 0;
    }

    case WM_SETCURSOR:
        // �J�[�\������ɔ�\���ɂ���
        ShowCursor(false);
        return true;
    case WM_MOUSEACTIVATE:
    {
        // �f�t�H���g�̃E�B���h�E�v���V�[�W���ɏ�����C����
        return DefWindowProc(hwnd, umsg, wparam, lparam);
    }
    // �A�v���P�[�V�������t�H�[�J�X�����������ɃL�[���͂����Z�b�g�����悤�ɂ���
    case WM_KILLFOCUS:
        m_Input->Initialize();
        return 0;
    default:
    {
        return DefWindowProc(hwnd, umsg, wparam, lparam);
    }
    }
}

void Game::InitializeWindows(int& screenWidth, int& screenHeight)
{
    WNDCLASSEX wc;

    m_applicationName = L"MyGame";
    m_hinstance = GetModuleHandle(NULL);

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

    RegisterClassEx(&wc);

    screenWidth = 1280;
    screenHeight = 720;

    m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

    // �����x�}�E�X���͂�o�^ 
    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01; // Generic Desktop
    rid.usUsage = 0x02;     // Mouse
    rid.dwFlags = RIDEV_NOLEGACY;
    rid.hwndTarget = m_hwnd; // �쐬�����E�B���h�E�n���h�����w��
    RegisterRawInputDevices(&rid, 1, sizeof(rid));

    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
    SetFocus(m_hwnd);

    // �J�[�\�����\���ɂ���
    ShowCursor(false);
}

void Game::ShutdownWindows()
{
    // �J�[�\�����ĕ\��
    ShowCursor(true);

    DestroyWindow(m_hwnd);
    m_hwnd = NULL;

    UnregisterClass(m_applicationName, m_hinstance);
    m_hinstance = NULL;

    ApplicationHandle = NULL;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
    switch (umessage)
    {
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }
    case WM_CLOSE:
    {
        PostQuitMessage(0);
        return 0;
    }
    default:
    {
        return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
    }
    }
}