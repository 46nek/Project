#include "ResultScene.h"
#include "framework.h"
#include "Game.h"
#include "AssetPaths.h"

ResultScene::ResultScene()
	: m_clearTextScale(1.0f), m_pressEnterScale(1.0f)
{
}

ResultScene::~ResultScene()
{
}

bool ResultScene::Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine)
{
	m_graphicsDevice = graphicsDevice;
	m_input = input;
	m_audioEngine = audioEngine;

	ID3D11Device* device = m_graphicsDevice->GetDevice();
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_graphicsDevice->GetDeviceContext());

	m_background = std::make_unique<Sprite>();
	if (!m_background->Initialize(device, AssetPaths::TEX_BACKGROUND)) return false;

	m_clearText = std::make_unique<Sprite>();
	if (!m_clearText->Initialize(device, AssetPaths::TEX_TITLE)) return false;

	m_pressEnter = std::make_unique<Sprite>();
	if (!m_pressEnter->Initialize(device, AssetPaths::TEX_BUTTON)) return false;

	// スケール調整
	const float desiredTextWidth = 600.0f;
	const float desiredButtonWidth = 400.0f;

	if (m_clearText->GetWidth() > 0)
	{
		m_clearTextScale = desiredTextWidth / m_clearText->GetWidth();
	}
	if (m_pressEnter->GetWidth() > 0)
	{
		m_pressEnterScale = desiredButtonWidth / m_pressEnter->GetWidth();
	}

	return true;
}

void ResultScene::Shutdown()
{
	m_background->Shutdown();
	m_clearText->Shutdown();
	m_pressEnter->Shutdown();
}

void ResultScene::Update(float deltaTime)
{
	// エンターキーでタイトルへ戻る
	if (m_input->IsKeyPressed(VK_RETURN))
	{
		m_nextScene = SceneState::Title;
	}
}

void ResultScene::Render()
{
	m_graphicsDevice->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
	m_graphicsDevice->GetSwapChain()->TurnZBufferOff(m_graphicsDevice->GetDeviceContext());

	m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, m_graphicsDevice->GetAlphaBlendState());

	RECT screenRect = { 0, 0, Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT };
	m_background->RenderFill(m_spriteBatch.get(), screenRect);

	// 中央に表示
	m_clearText->Render(m_spriteBatch.get(), { Game::SCREEN_WIDTH / 2.0f, 250.0f }, m_clearTextScale);
	m_pressEnter->Render(m_spriteBatch.get(), { Game::SCREEN_WIDTH / 2.0f, 600.0f }, m_pressEnterScale);

	m_spriteBatch->End();

	m_graphicsDevice->GetSwapChain()->TurnZBufferOn(m_graphicsDevice->GetDeviceContext());
	m_graphicsDevice->EndScene();
}