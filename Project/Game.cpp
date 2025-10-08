#include "Game.h"

Game::Game() : m_isMessageBoxActive(false)
{
}

Game::~Game()
{
}

bool Game::Initialize(HINSTANCE hInstance)
{
    m_input = std::make_unique<Input>();
    m_input->Initialize();

    m_window = std::make_unique<Window>(L"MyGame", SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!m_window->Initialize(hInstance, m_input.get()))
    {
        return false;
    }

    m_direct3D = std::make_unique<Direct3D>();
    if (!m_direct3D->Initialize(m_window->GetHwnd(), SCREEN_WIDTH, SCREEN_HEIGHT))
    {
        MessageBox(m_window->GetHwnd(), L"Could not initialize Direct3D.", L"Error", MB_OK);
        return false;
    }

    m_sceneManager = std::make_unique<SceneManager>();
    if (!m_sceneManager->Initialize(m_direct3D.get(), m_input.get()))
    {
        return false;
    }

    return true;
}

void Game::Shutdown()
{
    if (m_sceneManager) m_sceneManager->Shutdown();
    if (m_direct3D) m_direct3D->Shutdown();
    if (m_window) m_window->Shutdown();
}

void Game::Run()
{
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            if (!Frame())
            {
                break;
            }
        }
    }
}

bool Game::Frame()
{
    if (m_input->IsKeyPressed(VK_ESCAPE))
    {
        m_isMessageBoxActive = true;
        ShowCursor(true);
        int result = MessageBox(m_window->GetHwnd(), L"ゲームを終了しますか？", L"終了の確認", MB_YESNO | MB_ICONQUESTION);
        ShowCursor(false);
        m_isMessageBoxActive = false;

        if (result == IDYES)
        {
            return false;
        }
    }

    m_sceneManager->Update(0.0f);
    m_sceneManager->Render();
    m_input->Frame();

    return true;
}