#include "TitleScene.h"

TitleScene::TitleScene()
{
}

TitleScene::~TitleScene()
{
}

bool TitleScene::Initialize(Direct3D* d3d, Input* input)
{
	m_D3D = d3d;
	m_Input = input;

	// �w�i�̏�����
	m_background = std::make_unique<Sprite>();
	if (!m_background->Initialize(d3d->GetDevice(), L"background.png")) // ���̃t�@�C����
	{
		return false;
	}

	// �^�C�g�����S�̏�����
	m_titleLogo = std::make_unique<Sprite>();
	if (!m_titleLogo->Initialize(d3d->GetDevice(), L"title.png")) // ���̃t�@�C����
	{
		return false;
	}

	// Press Enter�e�L�X�g�̏�����
	m_pressEnter = std::make_unique<Sprite>();
	if (!m_pressEnter->Initialize(d3d->GetDevice(), L"button.png")) // ���̃t�@�C����
	{
		return false;
	}

	return true;
}

void TitleScene::Shutdown()
{
	if (m_background)
	{
		m_background->Shutdown();
	}
	if (m_titleLogo)
	{
		m_titleLogo->Shutdown();
	}
	if (m_pressEnter)
	{
		m_pressEnter->Shutdown();
	}
}

void TitleScene::Update(float deltaTime)
{
	// Enter�L�[�������ꂽ��Q�[���V�[���ɑJ��
	if (m_Input->IsKeyDown(VK_RETURN))
	{
		m_nextScene = SceneState::Game;
	}
}

void TitleScene::Render()
{
	// �w�i�����ŃN���A
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	m_D3D->Begin2D(); // 2D�`��J�n

	// �e�X�v���C�g���L���ȏꍇ�̂ݕ`�悷��
	if (m_background)
	{
		m_background->Render(m_D3D->GetSpriteBatch(), { 1280.0f / 2.0f, 720.0f / 2.0f });
	}
	if (m_titleLogo)
	{
		m_titleLogo->Render(m_D3D->GetSpriteBatch(), { 1280.0f / 2.0f, 200.0f });
	}
	if (m_pressEnter)
	{
		m_pressEnter->Render(m_D3D->GetSpriteBatch(), { 1280.0f / 2.0f, 600.0f });
	}

	m_D3D->End2D(); // 2D�`��I��

	m_D3D->EndScene();
}