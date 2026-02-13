#include "Game.h"
#include <comdef.h>

Game::Game() : m_isPaused(false) {
}

Game::~Game() {
}

void Game::SetPaused(bool isPaused) {
	m_isPaused = isPaused;

	if (m_isPaused) {
		m_input->SetCursorVisible(true);

		// 繧ｪ繝ｼ繝・ぅ繧ｪ繧剃ｸ譎ょ●豁｢
		if (m_audioEngine) {
			m_audioEngine->Suspend();
		}
	}
	else {
		// 繝昴・繧ｺ隗｣髯､
		if (m_input->IsCursorLocked()) {
			m_input->SetCursorVisible(false);
		}

		// 繧ｪ繝ｼ繝・ぅ繧ｪ繧貞・髢・
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

	m_window = std::make_unique<Window>(L"OBSCURA", SCREEN_WIDTH, SCREEN_HEIGHT);
	if (!m_window->Initialize(hInstance, m_input.get())) {
		return false;
	}

	m_graphicsDevice = std::make_unique<GraphicsDevice>();
	if (!m_graphicsDevice->Initialize(m_window->GetHwnd(), SCREEN_WIDTH, SCREEN_HEIGHT)) {
		MessageBox(m_window->GetHwnd(), L"Could not initialize Graphics Device.", L"Error", MB_OK);
		return false;
	}

	m_audioEngine = std::make_unique<DirectX::AudioEngine>(DirectX::AudioEngine_Default);

	if (m_audioEngine) {
		m_audioEngine->SetMasterVolume(m_settings.volume);
	}

	m_sceneManager = std::make_unique<SceneManager>();
	if (!m_sceneManager->Initialize(m_graphicsDevice.get(), m_input.get(), m_audioEngine.get())) {
		return false;
	}

	m_timer = std::make_unique<Timer>();
	if (!m_timer->Initialize()) {
		return false;
	}

	// 蛻晄悄險ｭ螳壹ｒ蜿肴丐
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
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
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

	if (!m_isPaused) {
		// Input縺ｮ譖ｴ譁ｰ・医％縺薙〒繧ｫ繝ｼ繧ｽ繝ｫ荳ｭ螟ｮ蝗ｺ螳壹′螳溯｡後＆繧後∪縺呻ｼ・
		// 繧ｦ繧｣繝ｳ繝峨え繝上Φ繝峨Ν繧呈ｸ｡縺励※縲∽ｸｭ蠢・ｺｧ讓吶ｒ險育ｮ励＆縺帙∪縺・
		m_input->Update(m_window->GetHwnd());

		m_sceneManager->Update(m_timer->GetDeltaTime());
	}
	else {
		// 繝昴・繧ｺ荳ｭ縺ｯ繧ｫ繝ｼ繧ｽ繝ｫ繝ｭ繝・け蜃ｦ逅・ｼ・nput::Update・峨ｒ蜻ｼ縺ｰ縺ｪ縺・％縺ｨ縺ｧ縲・
		// 繝槭え繧ｹ繧ｫ繝ｼ繧ｽ繝ｫ繧定・逕ｱ縺ｫ蜍輔°縺帙ｋ繧医≧縺ｫ縺励∪縺吶・
	}

	m_input->EndFrame();
	return true;
}

void Game::Render() {
	m_sceneManager->Render();
}
