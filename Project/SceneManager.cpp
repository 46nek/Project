#include "SceneManager.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "LoadingScene.h"
#include "ResultScene.h"

SceneManager::SceneManager()
	: m_currentScene(nullptr),
	m_graphicsDevice(nullptr),
	m_input(nullptr),
	m_audioEngine(nullptr)
{
}

SceneManager::~SceneManager()
{
}

bool SceneManager::Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine)
{
	m_graphicsDevice = graphicsDevice;
	m_input = input;
	m_audioEngine = audioEngine;

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
	if (nextState == SceneState::None)
	{
		return true;
	}

	// LoadingSceneからGameSceneへの遷移（データの受け渡しが必要な場合）
	if (nextState == SceneState::Game)
	{
		LoadingScene* loadingScene = dynamic_cast<LoadingScene*>(m_currentScene.get());
		if (loadingScene)
		{
			// ローディング済みのGameSceneの所有権を取得
			std::unique_ptr<Scene> nextScene = loadingScene->GetGameScene();

			// 現在のシーンを破棄
			m_currentScene->Shutdown();
			m_currentScene = nullptr;

			// GameSceneをセット
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
		m_currentScene = std::make_unique<GameScene>();
		break;
	case SceneState::Result:
		m_currentScene = std::make_unique<ResultScene>();
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