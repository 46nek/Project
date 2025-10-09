#include "GameScene.h"
#include "framework.h"

GameScene::GameScene()
{
}

GameScene::~GameScene()
{
}

bool GameScene::Initialize(Direct3D* d3d, Input* input)
{
	DbgPrint(L"GameScene Initialize started...");

	m_D3D = d3d;
	m_Input = input;

	DbgPrint(L"--> Creating Camera...");
	m_Camera = std::make_unique<Camera>();

	DbgPrint(L"--> Creating MazeGenerator...");
	m_mazeGenerator = std::make_unique<MazeGenerator>();
	DbgPrint(L"--> Generating Maze (21x21)...");
	m_mazeGenerator->Generate(21, 21);
	DbgPrint(L"--> Maze Generation complete.");

	const auto& mazeData = m_mazeGenerator->GetMazeData();
	const int mazeHeight = static_cast<int>(mazeData.size());
	const int mazeWidth = static_cast<int>(mazeData[0].size());

	DbgPrint(L"--> Setting Camera position...");
	// カメラの初期位置を迷路の道の上に調整 (以前の修正を適用)
	bool startFound = false;
	for (int y = 0; y < mazeHeight && !startFound; ++y) {
		for (int x = 0; x < mazeWidth && !startFound; ++x) {
			if (mazeData[y][x] == MazeGenerator::Path) {
				m_Camera->SetPosition(static_cast<float>(x) * 2.0f, 2.0f, static_cast<float>(y) * 2.0f);
				startFound = true;
			}
		}
	}
	if (!startFound) {
		m_Camera->SetPosition(1.0f * 2.0f, 1.0f, 1.0f * 2.0f); // フォールバック
	}


	DbgPrint(L"--> Initializing wall model...");
	m_wallModel = std::make_unique<Model>();
	if (!m_wallModel->Initialize(m_D3D->GetDevice(), "Assets/cube.obj"))
	{
		DbgPrint(L"!!!!!! FAILED to initialize wall model.");
		return false;
	}
	if (!m_wallModel->LoadTexture(m_D3D->GetDevice(), L"Assets/wall.jpg"))
	{
		DbgPrint(L"!!!!!! FAILED to load wall texture.");
		// テクスチャがなくても続行する場合はreturnしない
	}
	DbgPrint(L"--> Wall model initialized successfully.");

	// 壁の位置情報を格納
	for (int y = 0; y < mazeHeight; ++y)
	{
		for (int x = 0; x < mazeWidth; ++x)
		{
			if (mazeData[y][x] == MazeGenerator::Wall)
			{
				m_wallPositions.emplace_back(x * 2.0f, 2.0f, y * 2.0f);
			}
		}
	}
	DbgPrint(L"--> Wall positions stored.");

	DbgPrint(L"--> Initializing floor model...");
	// 床モデルの読み込みと配置
	m_floorModel = std::make_unique<Model>();
	if (!m_floorModel->Initialize(m_D3D->GetDevice(), "Assets/cube.obj"))
	{
		DbgPrint(L"!!!!!! FAILED to initialize floor model.");
		return false;
	}
	if (!m_floorModel->LoadTexture(m_D3D->GetDevice(), L"Assets/floor.png"))
	{
		DbgPrint(L"!!!!!! FAILED to load floor texture.");
	}
	DbgPrint(L"--> Floor model initialized successfully.");

	// 床を迷路の中央に配置し、迷路全体を覆うように拡大
	m_floorModel->SetPosition(static_cast<float>(mazeWidth - 1), 0.0f, static_cast<float>(mazeHeight - 1));
	m_floorModel->SetScale(static_cast<float>(mazeWidth), 1.0f, static_cast<float>(mazeHeight));
	DbgPrint(L"--> Floor model position and scale set.");


	DbgPrint(L"GameScene Initialize finished successfully.");
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