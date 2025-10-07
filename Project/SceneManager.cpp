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

bool SceneManager::ChangeScene(SceneState nextState)
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
		return false; // ���m�̃V�[����ԂȂ玸�s
	}

	// �V�����V�[�������������A���̌��ʂ�Ԃ�
	if (m_currentScene)
	{
		if (!m_currentScene->Initialize(m_D3D, m_Input))
		{
			return false; // �������Ɏ��s������false��Ԃ�
		}
	}
	else
	{
		return false; // �V�[���̍쐬�Ɏ��s
	}

	return true; // ���ׂĐ���������true��Ԃ�
}