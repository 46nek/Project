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
	// Enterキーが押されたらゲームシーンに遷移
	if (m_Input->IsKeyDown(VK_RETURN))
	{
		m_nextScene = SceneState::Game;
	}
}

void TitleScene::Render()
{
	// 背景を黒でクリア
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// ここにタイトルロゴやメニューなどを描画するコードを追加できます

	m_D3D->EndScene();
}