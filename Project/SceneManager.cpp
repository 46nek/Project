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
	// �ŏ��̃V�[���Ƃ��ă^�C�g���V�[����ݒ�
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

		// �V�[���̐؂�ւ��`�F�b�N
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
	// ���݂̃V�[�������
	if (m_currentScene)
	{
		m_currentScene->Shutdown();
		m_currentScene.reset();
	}

	// �V�����V�[�����쐬
	switch (nextState)
	{
	case SceneState::Title:
		m_currentScene = std::make_unique<TitleScene>();
		break;
	case SceneState::Game:
		m_currentScene = std::make_unique<GameScene>();
		break;
	default:
		// ���m�̃V�[�����
		return;
	}

	// �V�����V�[����������
	if (m_currentScene)
	{
		m_currentScene->Initialize(m_D3D, m_Input);
	}
}