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

	// 背景の初期化
	m_background = std::make_unique<Sprite>();
	if (!m_background->Initialize(d3d->GetDevice(), L"background.png")) // 仮のファイル名
	{
		return false;
	}

	// タイトルロゴの初期化
	m_titleLogo = std::make_unique<Sprite>();
	if (!m_titleLogo->Initialize(d3d->GetDevice(), L"title.png")) // 仮のファイル名
	{
		return false;
	}

	// Press Enterテキストの初期化
	m_pressEnter = std::make_unique<Sprite>();
	if (!m_pressEnter->Initialize(d3d->GetDevice(), L"button.png")) // 仮のファイル名
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

	m_D3D->Begin2D(); // 2D描画開始

	// 各スプライトが有効な場合のみ描画する
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

	m_D3D->End2D(); // 2D描画終了

	m_D3D->EndScene();
}