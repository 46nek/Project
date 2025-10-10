#include "TitleScene.h"
#include "framework.h"
#include "Game.h"
#include "AssetLoader.h"

TitleScene::TitleScene() {}
TitleScene::~TitleScene() {}

bool TitleScene::Initialize(GraphicsDevice* graphicsDevice, Input* input)
{
    m_graphicsDevice = graphicsDevice;
    m_input = input;

    ID3D11Device* device = m_graphicsDevice->GetDevice();

    m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_graphicsDevice->GetDeviceContext());

    m_background = std::make_unique<Sprite>();
    if (!m_background->Initialize(device, L"../Assets/background.png")) return false;

    m_titleLogo = std::make_unique<Sprite>();
    if (!m_titleLogo->Initialize(device, L"../Assets/title.png")) return false;

    m_pressEnter = std::make_unique<Sprite>();
    if (!m_pressEnter->Initialize(device, L"../Assets/button.png")) return false;

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

    m_spriteBatch->Begin();

    RECT screenRect = { 0, 0, Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT };
    m_background->RenderFill(m_spriteBatch.get(), screenRect);
    m_titleLogo->Render(m_spriteBatch.get(), { Game::SCREEN_WIDTH / 2.0f, 200.0f });
    m_pressEnter->Render(m_spriteBatch.get(), { Game::SCREEN_WIDTH / 2.0f, 600.0f });

    m_spriteBatch->End();

    m_graphicsDevice->GetSwapChain()->TurnZBufferOn(m_graphicsDevice->GetDeviceContext());
    m_graphicsDevice->EndScene();
}