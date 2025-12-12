#include "Game.h"
#include <comdef.h>

// Gameのコンストラクタ
// m_cursorLockEnabled は Inputクラス側で管理するため削除
Game::Game() : m_isPaused(false) {
}

Game::~Game() {
}

void Game::SetPaused(bool isPaused) {
	m_isPaused = isPaused;

	if (m_isPaused) {
		// ポーズ中: カーソルを表示する
		// (ロックフラグ自体は変更せず、一時的に表示だけ許可するイメージでも良いが、
		//  ここではシンプルに「ロックを一時解除」ではなく「表示」だけ制御します)
		m_input->SetCursorVisible(true);

		// オーディオを一時停止
		if (m_audioEngine) {
			m_audioEngine->Suspend();
		}
	}
	else {
		// ポーズ解除
		// 元々ロック状態（ゲーム中）だったなら、カーソルを再度非表示にする
		if (m_input->IsCursorLocked()) {
			m_input->SetCursorVisible(false);
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

	// 初期設定を反映
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
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) { // NULL -> nullptr
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
	// カーソルロックが有効（＝ゲームプレイ中）の場合のみポーズ可能にする
	if (m_input->IsKeyPressed(VK_ESCAPE)) {
		if (m_input->IsCursorLocked()) {
			SetPaused(!m_isPaused);
		}
	}

	if (!m_isPaused) {
		// Inputの更新（ここでカーソル中央固定が実行されます）
		// ウィンドウハンドルを渡して、中心位置を計算させます
		m_input->Update(m_window->GetHwnd());

		m_sceneManager->Update(m_timer->GetDeltaTime());
	}
	else {
		// ポーズ中はカーソルロック処理（Input::Update）を呼ばないことで、
		// マウスカーソルを自由に動かせるようにします。
	}

	m_input->EndFrame();
	return true;
}

void Game::Render() {
	m_sceneManager->Render();
}