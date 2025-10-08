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

	// 迷路生成
	m_mazeGenerator = std::make_unique<MazeGenerator>();
	m_mazeGenerator->Generate(2, 2); // 21x21の大きさの迷路を生成

	const auto& mazeData = m_mazeGenerator->GetMazeData();
	const int mazeHeight = static_cast<int>(mazeData.size());
	const int mazeWidth = static_cast<int>(mazeData[0].size());

	// スタート地点にカメラを配置
	// カメラの初期位置を迷路の道の上に調整
	m_Camera->SetPosition(1.0f * 2.0f, 1.0f, 0.0f * 2.0f);

	// 壁モデルを1度だけ読み込む
	m_wallModel = std::make_unique<Model>();
	if (!m_wallModel->Initialize(m_D3D->GetDevice(), "Assets/wall.obj"))
		{
		return false;
		}

	// 壁の位置情報を格納
	for (int y = 0; y < mazeHeight; ++y)
	{
		for (int x = 0; x < mazeWidth; ++x)
		{
			if (mazeData[y][x] == MazeGenerator::Wall)
			{
				m_wallPositions.emplace_back(x * 2.0f, 1.0f, y * 2.0f);
			}
		}
	}

	// 床モデルの読み込みと配置
	m_floorModel = std::make_unique<Model>();
	if (!m_floorModel->Initialize(m_D3D->GetDevice(), "Assets/floor.obj")) 
	{
		return false;
	}
	// 床を迷路の中央に配置し、迷路全体を覆うように拡大
	m_floorModel->SetPosition(static_cast<float>(mazeWidth - 1), 0.0f, static_cast<float>(mazeHeight - 1)); 
	m_floorModel->SetScale(static_cast<float>(mazeWidth), 1.0f, static_cast<float>(mazeHeight));

	return true;
}

void GameScene::Shutdown()
{
	if (m_wallModel)
	{
		m_wallModel->Shutdown();
	}
	m_wallPositions.clear();

	if (m_floorModel)
	{
		m_floorModel->Shutdown();
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
	m_D3D->BeginScene(0.1f, 0.1f, 0.1f, 1.0f); // 背景を少し暗く変更
	m_D3D->TurnZBufferOn();

	m_D3D->SetViewMatrix(m_Camera->GetViewMatrix());

	ID3D11DeviceContext* deviceContext = m_D3D->GetDeviceContext();

	deviceContext->IASetInputLayout(m_D3D->GetInputLayout());
	deviceContext->VSSetShader(m_D3D->GetVertexShader(), nullptr, 0);
	deviceContext->PSSetShader(m_D3D->GetPixelShader(), nullptr, 0);

	ID3D11SamplerState* samplerState = m_D3D->GetSamplerState();
	deviceContext->PSSetSamplers(0, 1, &samplerState);

	// 全ての壁を描画
	if (m_wallModel)
	{
		for (const auto& position : m_wallPositions)
		{
			m_wallModel->SetPosition(position.x, position.y, position.z);
			XMMATRIX worldMatrix = m_wallModel->GetWorldMatrix();
			m_D3D->SetWorldMatrix(worldMatrix);
			m_D3D->UpdateMatrixBuffer();
			m_wallModel->Render(deviceContext);
		}
	}

	// 床を描画
	if (m_floorModel)
	{
		XMMATRIX worldMatrix = m_floorModel->GetWorldMatrix();
		m_D3D->SetWorldMatrix(worldMatrix);
		m_D3D->UpdateMatrixBuffer();
		m_floorModel->Render(deviceContext);
	}

	m_D3D->EndScene();
}