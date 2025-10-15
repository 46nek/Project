#include "SceneManager.h"
#include "TitleScene.h"
#include "GameScene.h"

// �R���X�g���N�^�̏������q���X�g���C�����܂�
SceneManager::SceneManager() : m_currentScene(nullptr), m_graphicsDevice(nullptr), m_input(nullptr)
{
}

SceneManager::~SceneManager()
{
}

// Initialize�̈����� GraphicsDevice* �ɓ��ꂵ�܂�
bool SceneManager::Initialize(GraphicsDevice* graphicsDevice, Input* input)
{
	m_graphicsDevice = graphicsDevice;
	m_input = input; // �����o�[�ϐ��𐳂����ݒ肵�܂�
	// �ŏ��̃V�[���Ƃ��ă^�C�g���V�[����ݒ�
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
		// Initialize�̈����� m_graphicsDevice �� m_input �ɏC�����܂�
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