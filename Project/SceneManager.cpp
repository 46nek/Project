#include "SceneManager.h"
#include "TitleScene.h"
#include "GameScene.h"

// �R���X�g���N�^�̏������q���X�g���C��
SceneManager::SceneManager() : m_currentScene(nullptr), m_graphicsDevice(nullptr), m_input(nullptr)
{
}

SceneManager::~SceneManager()
{
}

// Initialize�̈����� GraphicsDevice* �ɓ���
bool SceneManager::Initialize(GraphicsDevice* graphicsDevice, Input* input)
{
    m_graphicsDevice = graphicsDevice;
    m_input = input;
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
        return false;
    }

    // �V�����V�[����������
    if (m_currentScene)
    {
        // Initialize�̈����� m_graphicsDevice �ɏC��
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