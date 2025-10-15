#include "SceneManager.h"
#include "TitleScene.h"
#include "GameScene.h"

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
	m_audioEngine = audioEngine; // <--- この行を追加
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
	if (m_currentScene)
	{
		m_currentScene->Shutdown();
	}

	switch (nextState)
	{
	case SceneState::Title:
		m_currentScene = std::make_unique<TitleScene>();
		break;
	case SceneState::Game:
		m_currentScene = std::make_unique<GameScene>();
		break;
	default:
		return false;
	}

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