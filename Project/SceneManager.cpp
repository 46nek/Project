#include "SceneManager.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "LoadingScene.h"

// �R���X�g���N�^�̏������q���X�g���C�����܂�
SceneManager::SceneManager() : m_currentScene(nullptr), m_graphicsDevice(nullptr), m_input(nullptr), m_audioEngine(nullptr)
{
}

SceneManager::~SceneManager()
{
}

// Initialize�̈������C�����Am_audioEngine�����������܂�
bool SceneManager::Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine)
{
	m_graphicsDevice = graphicsDevice;
	m_input = input;
	m_audioEngine = audioEngine; // <--- ���̍s��ǉ�
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
    // ���̃V�[����None�Ȃ牽�����Ȃ�
    if (nextState == SceneState::None)
    {
        return true;
    }

    // GameScene�ւ̑J�ڂ���ʂɏ���
    if (nextState == SceneState::Game)
    {
        // ���݂̃V�[����LoadingScene���`�F�b�N
        LoadingScene* loadingScene = dynamic_cast<LoadingScene*>(m_currentScene.get());
        if (loadingScene)
        {
            // ���[�f�B���O�ς݂�GameScene�̏��L�����擾
            std::unique_ptr<Scene> nextScene = loadingScene->GetGameScene();

            // ���݂̃V�[���iLoadingScene�j��j��
            m_currentScene->Shutdown();
            m_currentScene = nullptr;

            // ���̃V�[�����Z�b�g�i����GameScene�͏������ς݁j
            m_currentScene = std::move(nextScene);
            return true;
        }
    }

    // �ʏ�̃V�[���J��
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
        // LoadingScene������ɒ���GameScene�𐶐�����ꍇ
        m_currentScene = std::make_unique<GameScene>();
        break;
    default:
        return false;
    }

    // �V�����V�[����������
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