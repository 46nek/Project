#include "TitleScene.h"
#include "framework.h"
#include "Game.h"
#include "AssetLoader.h"

// ▼▼▼ コンストラクタに初期化を追加 ▼▼▼
TitleScene::TitleScene()
	: m_titleLogoScale(1.0f), m_pressEnterScale(1.0f)
{
}
// ▲▲▲ 追加ここまで ▲▲▲

TitleScene::~TitleScene() {}

bool TitleScene::Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine)
{
	m_graphicsDevice = graphicsDevice;
	m_input = input;
	m_audioEngine = audioEngine;

	ID3D11Device* device = m_graphicsDevice->GetDevice();

	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_graphicsDevice->GetDeviceContext());

	m_background = std::make_unique<Sprite>();
	if (!m_background->Initialize(device, L"Assets/background.png")) return false;

	m_titleLogo = std::make_unique<Sprite>();
	if (!m_titleLogo->Initialize(device, L"Assets/title.png")) return false;

	m_pressEnter = std::make_unique<Sprite>();
	if (!m_pressEnter->Initialize(device, L"Assets/button.png")) return false;

	// ▼▼▼ 以下を追加 ▼▼▼
	// 目標の幅を設定
	const float desiredTitleWidth = 800.0f;
	const float desiredButtonWidth = 400.0f;

	// 元の画像の幅から拡大率を計算
	if (m_titleLogo->GetWidth() > 0)
	{
		m_titleLogoScale = desiredTitleWidth / m_titleLogo->GetWidth();
	}
	if (m_pressEnter->GetWidth() > 0)
	{
		m_pressEnterScale = desiredButtonWidth / m_pressEnter->GetWidth();
	}
	// ▲▲▲ 追加ここまで ▲▲▲

	return true;
}

void TitleScene::Shutdown()
{
	m_background->Shutdown();
	m_titleLogo->Shutdown();
	m_pressEnter->Shutdown();
}

void TitleScene::Update(float deltaTime)
{
	if (m_input->IsKeyPressed(VK_RETURN)) {
		m_nextScene = SceneState::Game;
	}
}

void TitleScene::Render()
{
	m_graphicsDevice->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
	m_graphicsDevice->GetSwapChain()->TurnZBufferOff(m_graphicsDevice->GetDeviceContext());

	// SpriteBatchのBeginにアルファブレンド用のステートを渡す
	m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, m_graphicsDevice->GetAlphaBlendState());

	RECT screenRect = { 0, 0, Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT };
	m_background->RenderFill(m_spriteBatch.get(), screenRect);

	m_titleLogo->Render(m_spriteBatch.get(), { Game::SCREEN_WIDTH / 2.0f, 200.0f }, m_titleLogoScale);
	m_pressEnter->Render(m_spriteBatch.get(), { Game::SCREEN_WIDTH / 2.0f, 600.0f }, m_pressEnterScale);

	m_spriteBatch->End();

	m_graphicsDevice->GetSwapChain()->TurnZBufferOn(m_graphicsDevice->GetDeviceContext());
	m_graphicsDevice->EndScene();
}