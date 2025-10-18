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

    // GameSceneのインスタンスを作成
    m_gameScene = std::make_unique<GameScene>();

    // フォントの初期化
    HRESULT hr = FW1CreateFactory(FW1_VERSION, &m_fontFactory);
    if (FAILED(hr)) return false;

    // 'device' を 'm_graphicsDevice->GetDevice()' に修正
    hr = m_fontFactory->CreateFontWrapper(m_graphicsDevice->GetDevice(), L"Impact", &m_fontWrapper);
    if (FAILED(hr))
    {
        // 'device' を 'm_graphicsDevice->GetDevice()' に修正
        hr = m_fontFactory->CreateFontWrapper(m_graphicsDevice->GetDevice(), L"Arial", &m_fontWrapper);
        if (FAILED(hr)) return false;
    }

    return true;
}

void LoadingScene::Shutdown()
{
    if (m_fontWrapper) m_fontWrapper->Release();
    if (m_fontFactory) m_fontFactory->Release();
}

void LoadingScene::Update(float deltaTime)
{
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
        // すべての初期化が完了したので、SceneManagerにGameSceneを渡して遷移する
        m_nextScene = SceneState::Game;
        // SceneManager側でスマートポインタを管理するように所有権を移す
        // この処理はSceneManagerの修正時に実装します
        break;
    }
    m_loadingPhase++;
}

void LoadingScene::Render()
{
    m_graphicsDevice->BeginScene(0.0f, 0.0f, 0.1f, 1.0f);

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