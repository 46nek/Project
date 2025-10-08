#include "GameScene.h"

GameScene::GameScene()
{
}

GameScene::~GameScene()
{
}

bool GameScene::Initialize(Direct3D* d3d, Input* input)
{
	m_D3D = d3d;
	m_Input = input;

	m_Camera = std::make_unique<Camera>();
	m_Camera->SetPosition(0.0f, 0.0f, -5.0f);

	m_Model = std::make_unique<Model>();
	if (!m_Model->Initialize(m_D3D->GetDevice(), "Assets/test.obj"))
	{
		return false;
	}

	return true;
}

void GameScene::Shutdown()
{
	if (m_Model)
	{
		m_Model->Shutdown();
	}
}

void GameScene::Update(float deltaTime)
{
	HandleInput(deltaTime);
	UpdateCamera(deltaTime);
}

void GameScene::HandleInput(float deltaTime)
{
	if (m_Input->IsKeyDown('W')) m_Camera->MoveForward(deltaTime);
	if (m_Input->IsKeyDown('S')) m_Camera->MoveBackward(deltaTime);
	if (m_Input->IsKeyDown('A')) m_Camera->MoveLeft(deltaTime);
	if (m_Input->IsKeyDown('D')) m_Camera->MoveRight(deltaTime);
}

void GameScene::UpdateCamera(float deltaTime)
{
	int mouseX, mouseY;
	m_Input->GetMouseDelta(mouseX, mouseY);
	m_Camera->Turn(mouseX, mouseY, deltaTime);
	m_Camera->Update();
}


void GameScene::Render()
{
	m_D3D->BeginScene(0.39f, 0.58f, 0.93f, 1.0f);
	m_D3D->TurnZBufferOn();

	XMMATRIX worldMatrix = XMMatrixIdentity();
	m_D3D->SetWorldMatrix(worldMatrix);
	m_D3D->SetViewMatrix(m_Camera->GetViewMatrix());
	m_D3D->UpdateMatrixBuffer();

	ID3D11DeviceContext* deviceContext = m_D3D->GetDeviceContext();

	deviceContext->IASetInputLayout(m_D3D->GetInputLayout());
	deviceContext->VSSetShader(m_D3D->GetVertexShader(), nullptr, 0);
	deviceContext->PSSetShader(m_D3D->GetPixelShader(), nullptr, 0);

	ID3D11SamplerState* samplerState = m_D3D->GetSamplerState();
	deviceContext->PSSetSamplers(0, 1, &samplerState);

	m_Model->Render(deviceContext);

	m_D3D->EndScene();
}