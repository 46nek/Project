#include "Game.h"

// Game�̃R���X�g���N�^�ŁA�Q�[���J�n���Ƀ|�[�Y��Ԃɂ���
Game::Game() : m_isPaused(true)
{
}

Game::~Game() {}

// SetPaused�֐����A�J�[�\���\�����m���ɍs���悤�ɏC��
void Game::SetPaused(bool isPaused)
{
    m_isPaused = isPaused;
    if (m_isPaused)
    {
        // �J�[�\�����\�������܂� ShowCursor(true) ���Ăяo��������
        while (ShowCursor(true) < 0);
    }
    else
    {
        // �J�[�\������\���ɂȂ�܂� ShowCursor(false) ���Ăяo��������
        while (ShowCursor(false) >= 0);
    }
}

bool Game::IsPaused() const
{
    return m_isPaused;
}

bool Game::Initialize(HINSTANCE hInstance)
{
    m_input = std::make_unique<Input>();
    m_input->Initialize();

    m_window = std::make_unique<Window>(L"MyGame", SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!m_window->Initialize(hInstance, m_input.get())) {
        return false;
    }
    // SetPaused���Ăяo���ď����J�[�\����Ԃ��m�肳����
    SetPaused(m_isPaused);

    m_graphicsDevice = std::make_unique<GraphicsDevice>();
    if (!m_graphicsDevice->Initialize(m_window->GetHwnd(), SCREEN_WIDTH, SCREEN_HEIGHT)) {
        MessageBox(m_window->GetHwnd(), L"Could not initialize Graphics Device.", L"Error", MB_OK);
        return false;
    }

    m_sceneManager = std::make_unique<SceneManager>();
    if (!m_sceneManager->Initialize(m_graphicsDevice.get(), m_input.get())) {
        return false;
    }

    m_timer = std::make_unique<Timer>();
    if (!m_timer->Initialize()) {
        return false;
    }

    return true;
}

void Game::Shutdown()
{
    if (m_sceneManager) m_sceneManager->Shutdown();
    if (m_graphicsDevice) m_graphicsDevice->Shutdown();
    if (m_window) m_window->Shutdown();
}

void Game::Run()
{
    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            if (!Update()) {
                break;
            }
            Render();
        }
    }
}

bool Game::Update()
{
    m_timer->Tick();

    if (m_input->IsKeyPressed(VK_ESCAPE)) {
        SetPaused(!m_isPaused);
    }

    if (!m_isPaused)
    {
        m_sceneManager->Update(m_timer->GetDeltaTime());

        HWND hwnd = m_window->GetHwnd();
        if (GetFocus() == hwnd)
        {
            POINT centerPoint = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
            ClientToScreen(hwnd, &centerPoint);
            SetCursorPos(centerPoint.x, centerPoint.y);
        }
    }

    m_input->EndFrame();
    return true;
}

void Game::Render()
{
    m_sceneManager->Render();
}