#include "SceneManager.h"
#include "TitleScene.h"
#include "GameScene.h"

// コンストラクタの初期化子リストを修正します
SceneManager::SceneManager() : m_currentScene(nullptr), m_graphicsDevice(nullptr), m_input(nullptr)
{
}

SceneManager::~SceneManager()
{
}

// Initializeの引数を GraphicsDevice* に統一します
bool SceneManager::Initialize(GraphicsDevice* graphicsDevice, Input* input)
{
	m_graphicsDevice = graphicsDevice;
	m_input = input; // メンバー変数を正しく設定します
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
		// Initializeの引数を m_graphicsDevice と m_input に修正します
		if (!m_currentScene->Initialize(m_graphicsDevice, m_input))
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