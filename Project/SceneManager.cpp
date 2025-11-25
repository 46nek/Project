#include "SceneManager.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "LoadingScene.h"
#include "ResultScene.h"

// コンストラクタの初期化子リストを修正します
SceneManager::SceneManager() : m_currentScene(nullptr), m_graphicsDevice(nullptr), m_input(nullptr), m_audioEngine(nullptr)
{
}

SceneManager::~SceneManager()
{
}

// Initializeの引数を修正し、m_audioEngineを初期化します
bool SceneManager::Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine)
{
	m_graphicsDevice = graphicsDevice;
	m_input = input;
	m_audioEngine = audioEngine; 
	// 最初のシーンとしてタイトルシーンを設定
	return ChangeScene(SceneState::Title);
}

void SceneManager::Shutdown()
{
	if (m_currentScene)
	{
		m_currentScene->Shutdown();
		m_currentScene.reset();
	}
}

void SceneManager::Update(float deltaTime)
{
	if (m_currentScene)
	{
		m_currentScene->Update(deltaTime);
		SceneState nextScene = m_currentScene->GetNextScene();
		if (nextScene != SceneState::None)
		{
			ChangeScene(nextScene);
		}
	}
}

void SceneManager::Render()
{
	if (m_currentScene)
	{
		m_currentScene->Render();
	}
}

bool SceneManager::ChangeScene(SceneState nextState)
{
    // 次のシーンがNoneなら何もしない
    if (nextState == SceneState::None)
    {
        return true;
    }

    // GameSceneへの遷移を特別に処理
    if (nextState == SceneState::Game)
    {
        // 現在のシーンがLoadingSceneかチェック
        LoadingScene* loadingScene = dynamic_cast<LoadingScene*>(m_currentScene.get());
        if (loadingScene)
        {
            // ローディング済みのGameSceneの所有権を取得
            std::unique_ptr<Scene> nextScene = loadingScene->GetGameScene();

            // 現在のシーン（LoadingScene）を破棄
            m_currentScene->Shutdown();
            m_currentScene = nullptr;

            // 次のシーンをセット（このGameSceneは初期化済み）
            m_currentScene = std::move(nextScene);
            return true;
        }
    }

    // 通常のシーン遷移
    if (m_currentScene)
    {
        m_currentScene->Shutdown();
        m_currentScene = nullptr;
    }

    switch (nextState)
    {
    case SceneState::Title:
        m_currentScene = std::make_unique<TitleScene>();
        break;
    case SceneState::Loading:
        m_currentScene = std::make_unique<LoadingScene>();
        break;
    case SceneState::Game:
        // LoadingSceneを介さずに直接GameSceneを生成する場合
        m_currentScene = std::make_unique<GameScene>();
        break;
    case SceneState::Result: 
        m_currentScene = std::make_unique<ResultScene>();
        break;
    default:
        return false;
    }

    // 新しいシーンを初期化
    if (m_currentScene)
    {
        if (!m_currentScene->Initialize(m_graphicsDevice, m_input, m_audioEngine))
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    return true;
}