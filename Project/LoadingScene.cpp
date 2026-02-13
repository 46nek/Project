#include "LoadingScene.h"
#include "Game.h"

LoadingScene::LoadingScene()
	: m_loadingPhase(0), m_fontFactory(nullptr), m_fontWrapper(nullptr) {
}

LoadingScene::~LoadingScene() {
}

bool LoadingScene::Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine) {
	m_graphicsDevice = graphicsDevice;
	m_input = input;
	m_audioEngine = audioEngine;

	if (GameScene::s_transferInstance) {
		m_gameScene = std::move(GameScene::s_transferInstance);
		// 迺ｰ蠅・Phase1逶ｸ蠖・縺ｯ逕滓・貂医∩縺ｪ縺ｮ縺ｧ縲∵ｬ｡縺ｯUI(Phase2逶ｸ蠖・縺九ｉ
		m_loadingPhase = 1;
	}
	else {
		// 蠑輔″邯吶℃縺後↑縺・ｴ蜷茨ｼ医ョ繝舌ャ繧ｰ襍ｷ蜍輔↑縺ｩ・峨・騾壼ｸｸ騾壹ｊ菴懈・
		m_gameScene = std::make_unique<GameScene>();
		m_loadingPhase = 0;
	}

	// 繝輔か繝ｳ繝医・蛻晄悄蛹・
	HRESULT hr = FW1CreateFactory(FW1_VERSION, &m_fontFactory);
	if (FAILED(hr)) { return false; }

	hr = m_fontFactory->CreateFontWrapper(m_graphicsDevice->GetDevice(), L"Impact", &m_fontWrapper);
	if (FAILED(hr)) {
		hr = m_fontFactory->CreateFontWrapper(m_graphicsDevice->GetDevice(), L"Arial", &m_fontWrapper);
		if (FAILED(hr)) { return false; }
	}

	return true;
}

void LoadingScene::Shutdown() {
	if (m_fontWrapper) { m_fontWrapper->Release(); }
	if (m_fontFactory) { m_fontFactory->Release(); }

	// 繧ｷ繝ｼ繝ｳ邨ゆｺ・凾縺ｫ谺｡・・ameScene・峨∈貂｡縺・
	if (m_gameScene) {
		GameScene::s_transferInstance = std::move(m_gameScene);
	}
}

void LoadingScene::Update(float deltaTime) {
	// 繝ｭ繝ｼ繝我ｸｭ繧りレ譎ｯ縺ｮ繝ｩ繧､繝医ｄ貍泌・繧貞虚縺九☆
	if (m_gameScene) {
		m_gameScene->UpdateTitleLoop(deltaTime);
	}

	// 繝輔Ξ繝ｼ繝縺斐→縺ｫGameScene縺ｮ蛻晄悄蛹門・逅・ｒ・第ｮｵ髫弱★縺､騾ｲ繧√ｋ
	switch (m_loadingPhase) {
	case 0:
		// Phase 1: Environment
		if (!m_gameScene->InitializeEnvironment(m_graphicsDevice, m_input, m_audioEngine)) {
			// 繧ｨ繝ｩ繝ｼ蜃ｦ逅・
		}
		break;
	case 1:
		// Phase 2: UI
		if (!m_gameScene->InitializeUI()) {
			// 繧ｨ繝ｩ繝ｼ蜃ｦ逅・
		}
		break;
	case 2:
		// Phase 3: Game Objects
		if (!m_gameScene->InitializeGameObjects()) {
			// 繧ｨ繝ｩ繝ｼ蜃ｦ逅・
		}
		break;
	case 3:
		// Audio
		if (!m_gameScene->InitializeAudio()) {
			// 繧ｨ繝ｩ繝ｼ蜃ｦ逅・
		}
		break;
	case 4:
		// 縺吶∋縺ｦ螳御ｺ・
		m_gameScene->BeginOpening();

		// 縺吶∋縺ｦ縺ｮ蛻晄悄蛹悶′螳御ｺ・＠縺溘・縺ｧ縲ヾceneManager縺ｫGameScene繧呈ｸ｡縺励※驕ｷ遘ｻ縺吶ｋ
		m_nextScene = SceneState::Game;
		// move縺ｯShutdown縺ｧ陦後≧
		break;
	}
	m_loadingPhase++;
}

void LoadingScene::Render() {
	m_graphicsDevice->BeginScene(0.0f, 0.0f, 0.1f, 1.0f);

	if (m_gameScene) {
		// Z繝舌ャ繝輔ぃ繧丹N縺ｫ縺励※3D謠冗判
		m_graphicsDevice->GetSwapChain()->TurnZBufferOn(m_graphicsDevice->GetDeviceContext());
		m_gameScene->RenderStageOnly(); // 閭梧勹縺ｮ縺ｿ
		// Z繝舌ャ繝輔ぃ繧丹FF縺ｫ謌ｻ縺励※2D(譁・ｭ・謠冗判縺ｸ
		m_graphicsDevice->GetSwapChain()->TurnZBufferOff(m_graphicsDevice->GetDeviceContext());
	}

	if (m_fontWrapper) {
		m_fontWrapper->DrawString(
			m_graphicsDevice->GetDeviceContext(),
			L"Now Loading...",
			48.0f,
			(Game::SCREEN_WIDTH / 2.0f) - 150.0f,
			(Game::SCREEN_HEIGHT / 2.0f) - 24.0f,
			0xFFFFFFFF,
			FW1_RESTORESTATE
		);
	}

	m_graphicsDevice->EndScene();
}
