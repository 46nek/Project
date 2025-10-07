// Project/GameScene.cpp
#include "GameScene.h"

GameScene::GameScene()
{
	m_Camera = nullptr;
	m_Model = nullptr;
	m_Timer = nullptr;
}

GameScene::~GameScene()
{
}

bool GameScene::Initialize(Direct3D* d3d, Input* input)
{
	m_D3D = d3d;
	m_Input = input;

	// Timerオブジェクトを作成して初期化
	m_Timer = new Timer;
	if (!m_Timer) return false;
	if (!m_Timer->Initialize()) return false;

	// カメラオブジェクトを作成
	m_Camera = new Camera;
	if (!m_Camera) return false;
	m_Camera->SetPosition(0.0f, 0.0f, -5.0f);

	// Modelオブジェクトを作成
	m_Model = new Model;
	if (!m_Model) return false;
	if (!m_Model->Initialize(m_D3D->GetDevice())) return false;

	return true;
}

void GameScene::Shutdown()
{
	if (m_Model)
	{
		m_Model->Shutdown();
		delete m_Model;
		m_Model = nullptr;
	}
	if (m_Timer)
	{
		delete m_Timer;
		m_Timer = nullptr;
	}
	if (m_Camera)
	{
		delete m_Camera;
		m_Camera = nullptr;
	}
}

void GameScene::Update(float deltaTime)
{
	m_Timer->Frame();
	float dt = m_Timer->GetTime();

	int mouseX, mouseY;
	m_Input->GetMouseDelta(mouseX, mouseY);
	m_Camera->Turn(mouseX, mouseY, dt);

	if (m_Input->IsKeyDown('W')) m_Camera->MoveForward(dt);
	if (m_Input->IsKeyDown('S')) m_Camera->MoveBackward(dt);
	if (m_Input->IsKeyDown('A')) m_Camera->MoveLeft(dt);
	if (m_Input->IsKeyDown('D')) m_Camera->MoveRight(dt);

	m_Camera->Update();
}

void GameScene::Render()
{
	// 背景色を青でクリア
	m_D3D->BeginScene(0.39f, 0.58f, 0.93f, 1.0f);

	// D3Dに行列をセット
	XMMATRIX worldMatrix = XMMatrixIdentity();
	m_D3D->SetWorldMatrix(worldMatrix);
	m_D3D->SetViewMatrix(m_Camera->GetViewMatrix());
	m_D3D->UpdateMatrixBuffer();

	ID3D11DeviceContext* deviceContext = m_D3D->GetDeviceContext();

	// シェーダーとインプットレイアウトを設定
	deviceContext->IASetInputLayout(m_D3D->GetInputLayout());
	deviceContext->VSSetShader(m_D3D->GetVertexShader(), nullptr, 0);
	deviceContext->PSSetShader(m_D3D->GetPixelShader(), nullptr, 0);

	ID3D11SamplerState* samplerState = m_D3D->GetSamplerState();
	deviceContext->PSSetSamplers(0, 1, &samplerState);

	// モデルを描画
	m_Model->Render(deviceContext);
	deviceContext->DrawIndexed(m_Model->GetIndexCount(), 0, 0);

	m_D3D->EndScene();
}