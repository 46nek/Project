#include "Game.h"
#include <comdef.h>

// Gameのコンストラクタ
// タイトル画面から始まるため、ポーズは解除、カーソルロックは無効で初期化
Game::Game() : m_isPaused(false), m_cursorLockEnabled(false) {
}

Game::~Game() {
}

void Game::SetPaused(bool isPaused) {
	m_isPaused = isPaused;

	if (m_isPaused) {
		// ポーズ中: カーソルを表示する（メニュー操作のため）
		while (ShowCursor(true) < 0);

		// オーディオを一時停止
		if (m_audioEngine) {
			m_audioEngine->Suspend();
		}
	}
	else {
		// ポーズ解除（ゲームプレイ再開）
		// もしゲームプレイ中（カーソルロック有効）なら、カーソルを消す
		if (m_cursorLockEnabled) {
			while (ShowCursor(false) >= 0);
		}
		else {
			// タイトル画面などなら、カーソルを表示したままにする
			while (ShowCursor(true) < 0);
		}

		// オーディオを再開
		if (m_audioEngine) {
			m_audioEngine->Resume();
		}
	}
}

bool Game::IsPaused() const {
	return m_isPaused;
}

bool Game::Initialize(HINSTANCE hInstance) {
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr)) {
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

	// 初期状態の設定
	// ここでカーソル表示状態などが適用されます
	SetPaused(m_isPaused);

	return true;
}

void Game::Shutdown() {
	if (m_audioEngine) {
		m_audioEngine->Suspend();
	}
	m_audioEngine.reset();

	if (m_sceneManager) m_sceneManager->Shutdown();
	if (m_graphicsDevice) m_graphicsDevice->Shutdown();
	if (m_window) m_window->Shutdown();

	CoUninitialize();
}

void Game::Run() {
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

bool Game::Update() {
	m_timer->Tick();

	if (m_audioEngine && !m_audioEngine->Update()) {
		if (m_audioEngine->IsCriticalError()) {
		}
	}

	// ESCキーでポーズ切り替え
	// 「カーソルロックが有効（＝ゲームシーン）」の場合のみ、ESCでポーズ＆カーソル表示を行う
	if (m_input->IsKeyPressed(VK_ESCAPE)) {
		if (m_cursorLockEnabled) {
			SetPaused(!m_isPaused);
		}
	}

	if (!m_isPaused) {
		m_sceneManager->Update(m_timer->GetDeltaTime());

		// カーソルロックが有効で、かつポーズ中でない場合のみ、マウスを画面中央に固定する
		if (m_cursorLockEnabled) {
			HWND hwnd = m_window->GetHwnd();
			if (GetFocus() == hwnd) {
				POINT centerPoint = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
				ClientToScreen(hwnd, &centerPoint);
				SetCursorPos(centerPoint.x, centerPoint.y);
			}
		}
	}

	m_input->EndFrame();
	return true;
}

void Game::Render() {
	m_sceneManager->Render();
}