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
    // ウィンドウサイズを初期化
    m_screenWidth = 0;
    m_screenHeight = 0;

    // Windows APIを初期化
    InitializeWindows(m_screenWidth, m_screenHeight);

    // Inputオブジェクトを作成して初期化
    m_Input = new Input;
    if (!m_Input)
    {
        return false;
    }
    m_Input->Initialize();

    // カメラオブジェクトを作成
    m_Camera = new Camera;
    if (!m_Camera)
    {
        return false;
    }
    // カメラの初期位置を設定 (少し後ろに下げる)
    m_Camera->SetPosition(0.0f, 0.0f, -5.0f);

    // Direct3Dオブジェクトを作成
    m_D3D = new Direct3D;
    if (!m_D3D)
    {
        return false;
    }

    // Direct3Dを初期化
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

    // Direct3Dオブジェクトを解放
    if (m_D3D)
    {
        m_D3D->Shutdown();
        delete m_D3D;
        m_D3D = nullptr;
    }

    // ウィンドウを閉じる
    ShutdownWindows();
}

void Game::Run()
{
    MSG msg;
    bool result;

    ZeroMemory(&msg, sizeof(MSG));

    // メインループ
    while (true)
    {
        // ウィンドウメッセージがある場合、それを処理
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // WM_QUITメッセージを受け取ったらループを抜ける
            if (msg.message == WM_QUIT)
            {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            // メッセージがなければ、ゲームのフレームを処理
            result = Frame();
            if (!result)
            {
                // Frame()がfalseを返した場合もループを抜ける
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

    // --- 入力処理 ---
    // Wキーで前進
    if (m_Input->IsKeyDown('W'))
    {
        m_Camera->MoveForward();
    }
    // Sキーで後退
    if (m_Input->IsKeyDown('S'))
    {
        m_Camera->MoveBackward();
    }
    // Aキーで左へ
    if (m_Input->IsKeyDown('A'))
    {
        m_Camera->MoveLeft();
    }
    // Dキーで右へ
    if (m_Input->IsKeyDown('D'))
    {
        m_Camera->MoveRight();
    }

    // カメラを更新
    m_Camera->Update();

    // D3Dに行列をセット
    m_D3D->SetViewMatrix(m_Camera->GetViewMatrix());
    // WorldとProjectionはDirect3Dクラスで初期化済みなのでここではセット不要

    // 行列バッファを更新
    m_D3D->UpdateMatrixBuffer();

    // 背景色を青でクリア
    m_D3D->BeginScene(0.39f, 0.58f, 0.93f, 1.0f);

    m_D3D->GetDeviceContext()->Draw(3, 0); // 三角形を描画

    // バックバッファを画面に表示
    m_D3D->EndScene();

    // 次のフレームのために、フレームの最後にマウス移動量をリセット
    m_Input->Frame();

    return true;
}

LRESULT CALLBACK Game::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
    switch (umsg)
    {
        // 高精度マウス入力のメッセージ
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
        // キーが押されたメッセージ
    case WM_KEYDOWN:
    {
        m_Input->KeyDown((unsigned int)wparam);
        return 0;
    }

    // キーが離されたメッセージ
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
    
    // 高精度マウス入力を登録 
    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01; // Generic Desktop
    rid.usUsage = 0x02;     // Mouse
    rid.dwFlags = RIDEV_NOLEGACY;
    rid.hwndTarget = m_hwnd; // 作成したウィンドウハンドルを指定
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