#include "LoadingScene.h"
#include "Game.h"

LoadingScene::LoadingScene()
	: m_loadingPhase(0), m_fontFactory(nullptr), m_fontWrapper(nullptr)
{
}

LoadingScene::~LoadingScene()
{
}

bool LoadingScene::Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine)
{
	m_graphicsDevice = graphicsDevice;
	m_input = input;
	m_audioEngine = audioEngine;

	if (GameScene::s_transferInstance)
	{
		m_gameScene = std::move(GameScene::s_transferInstance);
		// Phase1(迷路生成)は終わっているので、次はPhase2から
		m_loadingPhase = 1;
	}
	else
	{
		// 引き継ぎがない場合（デバッグ起動など）は通常通り作成
		m_gameScene = std::make_unique<GameScene>();
		m_loadingPhase = 0;
	}

	// フォントの初期化
	HRESULT hr = FW1CreateFactory(FW1_VERSION, &m_fontFactory);
	if (FAILED(hr)) return false;

	hr = m_fontFactory->CreateFontWrapper(m_graphicsDevice->GetDevice(), L"Impact", &m_fontWrapper);
	if (FAILED(hr))
	{
		hr = m_fontFactory->CreateFontWrapper(m_graphicsDevice->GetDevice(), L"Arial", &m_fontWrapper);
		if (FAILED(hr)) return false;
	}

	return true;
}

void LoadingScene::Shutdown()
{
	if (m_fontWrapper) m_fontWrapper->Release();
	if (m_fontFactory) m_fontFactory->Release();

	// シーン終了時に次（GameScene）へ渡す
	// 修正: これでLoading画面の最後のフレームまでm_gameSceneが存在する
	if (m_gameScene)
	{
		GameScene::s_transferInstance = std::move(m_gameScene);
	}
}

void LoadingScene::Update(float deltaTime)
{
	// ロード中も背景のライトや演出を動かす
	if (m_gameScene)
	{
		m_gameScene->UpdateTitleLoop(deltaTime);
	}

	// フレームごとにGameSceneの初期化処理を１段階ずつ進める
	switch (m_loadingPhase)
	{
	case 0:
		if (!m_gameScene->InitializePhase1(m_graphicsDevice, m_input, m_audioEngine)) {
			// エラー処理
		}
		break;
	case 1:
		if (!m_gameScene->InitializePhase2()) {
			// エラー処理
		}
		break;
	case 2:
		if (!m_gameScene->InitializePhase3()) {
			// エラー処理
		}
		break;
	case 3:
		if (!m_gameScene->InitializePhase4()) {
			// エラー処理
		}
		break;
	case 4:
		if (!m_gameScene->InitializePhase5()) {
			// エラー処理
		}
		break;
	case 5:
		m_gameScene->BeginOpening();

		// すべての初期化が完了したので、SceneManagerにGameSceneを渡して遷移する
		m_nextScene = SceneState::Game;
		// moveはShutdownで行う
		break;
	}
	m_loadingPhase++;
}

void LoadingScene::Render()
{
	m_graphicsDevice->BeginScene(0.0f, 0.0f, 0.1f, 1.0f);

	if (m_gameScene)
	{
		// ZバッファをONにして3D描画
		m_graphicsDevice->GetSwapChain()->TurnZBufferOn(m_graphicsDevice->GetDeviceContext());
		m_gameScene->RenderStageOnly(); // 背景のみ
		// ZバッファをOFFに戻して2D(文字)描画へ
		m_graphicsDevice->GetSwapChain()->TurnZBufferOff(m_graphicsDevice->GetDeviceContext());
	}

	if (m_fontWrapper)
	{
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