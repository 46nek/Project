// Game.cpp (この内容で完全に置き換えてください)

#include "Game.h"
#include <comdef.h> // CoInitializeEx用

// Gameのコンストラクタで、ゲーム開始時にポーズ状態にする
Game::Game() : m_isPaused(true)
{
}

Game::~Game() {}

void Game::SetPaused(bool isPaused)
{
	m_isPaused = isPaused;
	if (m_isPaused)
	{
		// カーソルが表示されるまで ShowCursor(true) を呼び出し続ける
		while (ShowCursor(true) < 0);
		// ▼▼▼ オーディオを一時停止 ▼▼▼
		if (m_audioEngine)
		{
			m_audioEngine->Suspend();
		}
	}
	else
	{
		// カーソルが非表示になるまで ShowCursor(false) を呼び出し続ける
		while (ShowCursor(false) >= 0);
		// ▼▼▼ オーディオを再開 ▼▼▼
		if (m_audioEngine)
		{
			m_audioEngine->Resume();
		}
	}
}

bool Game::IsPaused() const
{
	return m_isPaused;
}

bool Game::Initialize(HINSTANCE hInstance)
{
	// ▼▼▼ COMライブラリを初期化 ▼▼▼
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		// エラー処理
		return false;
	}

	m_input = std::make_unique<Input>();
	m_input->Initialize();

	m_window = std::make_unique<Window>(L"The Unseen", SCREEN_WIDTH, SCREEN_HEIGHT);
	if (!m_window->Initialize(hInstance, m_input.get())) {
		return false;
	}

	m_graphicsDevice = std::make_unique<GraphicsDevice>();
	if (!m_graphicsDevice->Initialize(m_window->GetHwnd(), SCREEN_WIDTH, SCREEN_HEIGHT)) {
		MessageBox(m_window->GetHwnd(), L"Could not initialize Graphics Device.", L"Error", MB_OK);
		return false;
	}

	m_audioEngine = std::make_unique<DirectX::AudioEngine>(DirectX::AudioEngine_Default);

	m_sceneManager = std::make_unique<SceneManager>();
	if (!m_sceneManager->Initialize(m_graphicsDevice.get(), m_input.get(), m_audioEngine.get())) {
		return false;
	}

	m_timer = std::make_unique<Timer>();
	if (!m_timer->Initialize()) {
		return false;
	}

	// SetPausedを呼び出して初期カーソル状態とオーディオ状態を確定させる
	SetPaused(m_isPaused);

	return true;
}

void Game::Shutdown()
{
	if (m_audioEngine)
	{
		m_audioEngine->Suspend();
	}
	m_audioEngine.reset(); // スマートポインタを解放

	if (m_sceneManager) m_sceneManager->Shutdown();
	if (m_graphicsDevice) m_graphicsDevice->Shutdown();
	if (m_window) m_window->Shutdown();

	CoUninitialize();
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

	if (m_audioEngine && !m_audioEngine->Update())
	{
		// オーディオデバイスが失われた場合などのエラー処理
		if (m_audioEngine->IsCriticalError())
		{
			// エラーメッセージを表示するなど
		}
	}

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