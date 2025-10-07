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
	return true;
}

void TitleScene::Shutdown()
{
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

	// �����Ƀ^�C�g�����S�⃁�j���[�Ȃǂ�`�悷��R�[�h��ǉ��ł��܂�

	m_D3D->EndScene();
}