#include "Game.h"
#include <utility>

Game::Game()
{
    m_applicationName = nullptr;
    m_hinstance = nullptr;
    m_hwnd = nullptr;
    m_screenWidth = 0;
    m_screenHeight = 0;
    m_isMessageBoxActive = false;

    m_D3D = nullptr;
    m_Input = nullptr;
    m_SceneManager = nullptr; // 追加
    ApplicationHandle = this;
}

Game::~Game()
{
}

bool Game::Initialize()
{
    InitializeWindows(m_screenWidth, m_screenHeight);

    m_Input = std::make_unique<Input>();
    m_Input->Initialize();

    m_D3D = std::make_unique<Direct3D>();
    if (!m_D3D->Initialize(m_hwnd, m_screenWidth, m_screenHeight))
    {
        MessageBox(m_hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
        return false;
    }

    m_SceneManager = std::make_unique<SceneManager>();
    if (!m_SceneManager->Initialize(m_D3D.get(), m_Input.get())) return false;

    return true;
}

void Game::Shutdown()
{
    if (m_SceneManager) m_SceneManager->Shutdown();
    if (m_D3D) m_D3D->Shutdown();

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
    // ESCキーが押された瞬間を検知
    if (m_Input->IsKeyPressed(VK_ESCAPE))
    {
        m_isMessageBoxActive = true;
        // メッセージボックスの前にカーソルを表示する
        ShowCursor(true);

        // 確認メッセージボックスを表示
        int result = MessageBox(m_hwnd, L"ゲームを終了しますか？", L"終了の確認", MB_YESNO | MB_ICONQUESTION);

        // メッセージボックスが閉じたら、すぐにカーソルを非表示に戻す
        ShowCursor(false);
        m_isMessageBoxActive = false;

        // ユーザーが「はい」を選んだ場合
        if (result == IDYES)
        {
            return false; // ループを抜けてゲームを終了
        }
    }

    // 毎フレームの処理をSceneManagerに委譲
    m_SceneManager->Update(0.0f); // deltaTimeは各シーンのタイマーで管理
    m_SceneManager->Render();

    // 次のフレームのために、フレームの最後にマウス移動量をリセット
    m_Input->Frame();

    return true;
}

// MessageHandler, InitializeWindows, ShutdownWindows, WndProc は変更なし
// (ただし、Game::Frame()内の終了確認ロジックはGameSceneに移動したので削除)
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

    case WM_SETCURSOR:
        // メッセージボックスが表示中でなければ、カーソルを非表示にする
        if (!m_isMessageBoxActive)
        {
            ShowCursor(false);
            return true;
        }
        // メッセージボックス表示中は、OSの標準カーソル処理に完全に任せる
        return DefWindowProc(hwnd, umsg, wparam, lparam);

    case WM_MOUSEACTIVATE:
    {
        // デフォルトのウィンドウプロシージャに処理を任せる
        return DefWindowProc(hwnd, umsg, wparam, lparam);
    }
    // アプリケーションがフォーカスを失った時にキー入力がリセットされるようにする
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

    screenWidth = SCREEN_WIDTH;
    screenHeight = SCREEN_HEIGHT;

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

    // カーソルを非表示にする
    ShowCursor(false);
}

void Game::ShutdownWindows()
{
    // カーソルを再表示
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