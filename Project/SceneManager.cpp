#include "SceneManager.h"
#include "TitleScene.h"
#include "GameScene.h"

SceneManager::SceneManager()
{
	m_currentScene = nullptr;
	m_D3D = nullptr;
	m_Input = nullptr;
}

SceneManager::~SceneManager()
{
}

bool SceneManager::Initialize(Direct3D* d3d, Input* input)
{
	m_D3D = d3d;
	m_Input = input;
	// 最初のシーンとしてタイトルシーンを設定
	ChangeScene(SceneState::Title);
	return true;
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

		// シーンの切り替えチェック
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

void SceneManager::ChangeScene(SceneState nextState)
{
	// 現在のシーンを解放
	if (m_currentScene)
	{
		m_currentScene->Shutdown();
		m_currentScene.reset();
	}

	// 新しいシーンを作成
	switch (nextState)
	{
	case SceneState::Title:
		m_currentScene = std::make_unique<TitleScene>();
		break;
	case SceneState::Game:
		m_currentScene = std::make_unique<GameScene>();
		break;
	default:
		// 未知のシーン状態
		return;
	}

	// 新しいシーンを初期化
	if (m_currentScene)
	{
		m_currentScene->Initialize(m_D3D, m_Input);
	}
}