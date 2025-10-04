// Game.cpp
#include "Game.h"

Game::Game()
{
    m_D3D = nullptr;
    m_Camera = nullptr;
    m_Input = nullptr;
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
    if (!m_Input)
    {
        return false;
    }
    m_Input->Initialize();

    // �J�����I�u�W�F�N�g���쐬
    m_Camera = new Camera;
    if (!m_Camera)
    {
        return false;
    }
    // �J�����̏����ʒu��ݒ� (�������ɉ�����)
    m_Camera->SetPosition(0.0f, 0.0f, -5.0f);

    // Direct3D�I�u�W�F�N�g���쐬
    m_D3D = new Direct3D;
    if (!m_D3D)
    {
        return false;
    }

    // Direct3D��������
    bool result = m_D3D->Initialize(m_hwnd, m_screenWidth, m_screenHeight);
    if (!result)
    {
        MessageBox(m_hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
        return false;
    }

    return true;
}

void Game::Shutdown()
{
    if (m_Input)
    {
        delete m_Input;
        m_Input = nullptr;
    }
    if (m_Camera)
    {
        delete m_Camera;
        m_Camera = nullptr;
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
        // �E�B���h�E���b�Z�[�W������ꍇ�A���������
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // WM_QUIT���b�Z�[�W���󂯎�����烋�[�v�𔲂���
            if (msg.message == WM_QUIT)
            {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            // ���b�Z�[�W���Ȃ���΁A�Q�[���̃t���[��������
            result = Frame();
            if (!result)
            {
                // Frame()��false��Ԃ����ꍇ�����[�v�𔲂���
                break;
            }
        }
    }
}

bool Game::Frame()
{
    int mouseX, mouseY;
    m_Input->GetMouseDelta(mouseX, mouseY);
    m_Camera->Turn(mouseX, mouseY);

    // --- ���͏��� ---
    // W�L�[�őO�i
    if (m_Input->IsKeyDown('W'))
    {
        m_Camera->MoveForward();
    }
    // S�L�[�Ō��
    if (m_Input->IsKeyDown('S'))
    {
        m_Camera->MoveBackward();
    }
    // A�L�[�ō���
    if (m_Input->IsKeyDown('A'))
    {
        m_Camera->MoveLeft();
    }
    // D�L�[�ŉE��
    if (m_Input->IsKeyDown('D'))
    {
        m_Camera->MoveRight();
    }

    // �J�������X�V
    m_Camera->Update();

    // D3D�ɍs����Z�b�g
    m_D3D->SetViewMatrix(m_Camera->GetViewMatrix());
    // World��Projection��Direct3D�N���X�ŏ������ς݂Ȃ̂ł����ł̓Z�b�g�s�v

    // �s��o�b�t�@���X�V
    m_D3D->UpdateMatrixBuffer();

    // �w�i�F��ŃN���A
    m_D3D->BeginScene(0.39f, 0.58f, 0.93f, 1.0f);

    m_D3D->GetDeviceContext()->Draw(3, 0); // �O�p�`��`��

    // �o�b�N�o�b�t�@����ʂɕ\��
    m_D3D->EndScene();

    // ���̃t���[���̂��߂ɁA�t���[���̍Ō�Ƀ}�E�X�ړ��ʂ����Z�b�g
    m_Input->Frame();

    return true;
}

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
}

void Game::ShutdownWindows()
{
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