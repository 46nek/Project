#include "GameScene.h"
#include "framework.h"
#include <cmath>

GameScene::GameScene()
{
}

GameScene::~GameScene()
{
}

void GameScene::InitializeLights()
{
	m_lights.clear();

	// ���C�g1: �v���C���[��Ǐ]���锒���|�C���g���C�g
	Light playerLight;
	playerLight.Enabled = true;
	playerLight.Type = PointLight;
	playerLight.Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	playerLight.Intensity = 1.0f; // 2.0f ����ύX
	playerLight.Range = 10.0f;
	playerLight.Attenuation = XMFLOAT3(0.5f, 0.2f, 0.0f);
	m_lights.push_back(playerLight);

	// ���C�g2: ���H�����񂷂�Ԃ��|�C���g���C�g
	Light patrolLight;
	patrolLight.Enabled = true;
	patrolLight.Type = PointLight;
	patrolLight.Color = XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f);
	patrolLight.Intensity = 1.5f; // 3.0f ����ύX
	patrolLight.Range = 15.0f;
	patrolLight.Attenuation = XMFLOAT3(0.5f, 0.1f, 0.0f);
	m_lights.push_back(patrolLight);

	// ���C�g3: �_�ł���X�|�b�g���C�g
	Light flickeringSpotLight;
	flickeringSpotLight.Enabled = true;
	flickeringSpotLight.Type = SpotLight;
	flickeringSpotLight.Color = XMFLOAT4(1.0f, 1.0f, 0.5f, 1.0f);
	flickeringSpotLight.Position = XMFLOAT3(10.0f, 5.0f, 10.0f);
	flickeringSpotLight.Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	flickeringSpotLight.Intensity = 2.5f; // 5.0f ����ύX
	flickeringSpotLight.Range = 20.0f;
	flickeringSpotLight.SpotAngle = 0.95f; // �R�T�C���l
	flickeringSpotLight.Attenuation = XMFLOAT3(0.2f, 0.2f, 0.0f);
	m_lights.push_back(flickeringSpotLight);
}

bool GameScene::Initialize(Direct3D* d3d, Input* input)
{
	DbgPrint(L"GameScene Initialize started...");

	m_D3D = d3d;
	m_Input = input;

	constexpr float pathWidth = 3.0f;

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
	bool startFound = false;
	for (int y = 0; y < mazeHeight && !startFound; ++y) {
		for (int x = 0; x < mazeWidth && !startFound; ++x) {
			if (mazeData[y][x] == MazeGenerator::Path) {
				// �J�����̍����𒲐�
				m_Camera->SetPosition(static_cast<float>(x) * pathWidth, 1.5f, static_cast<float>(y) * pathWidth);
				startFound = true;
			}
		}
	}
	if (!startFound) {
		m_Camera->SetPosition(1.0f * pathWidth, 1.5f, 1.0f * pathWidth); // �t�H�[���o�b�N
	}

	DbgPrint(L"--> Initializing wall model...");
	// Model�N���X�̐ÓI���\�b�h���g���ĕǃ��f���𐶐�
	m_wallModel.reset(Model::CreateModelFromMaze(m_D3D->GetDevice(), mazeData, pathWidth));
	if (!m_wallModel)
	{
		DbgPrint(L"!!!!!! FAILED to initialize wall model.");
		return false;
	}
	if (!m_wallModel->LoadTexture(m_D3D->GetDevice(), L"Assets/wall.png"))
	{
		DbgPrint(L"!!!!!! FAILED to load wall texture.");
	}
	DbgPrint(L"--> Wall model initialized successfully.");

	DbgPrint(L"--> Initializing floor model...");
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
	m_floorModel->SetPosition(static_cast<float>(mazeWidth - 1) / 2.0f * pathWidth, -1.0f, static_cast<float>(mazeHeight - 1) / 2.0f * pathWidth);
	m_floorModel->SetScale(static_cast<float>(mazeWidth) / 2.0f * pathWidth, 1.0f, static_cast<float>(mazeHeight) / 2.0f * pathWidth);
	DbgPrint(L"--> Floor model initialized successfully.");

	InitializeLights();

	DbgPrint(L"GameScene Initialize finished successfully.");
	return true;
}

void GameScene::Shutdown()
{
}

void GameScene::Update(float deltaTime)
{
	HandleInput(deltaTime);
	UpdateCamera(deltaTime);

	// ���C�g�̍X�V�������������L�q����Ă��邩�m�F���Ă�������
	if (!m_lights.empty()) // m_lights����łȂ����Ƃ��m�F
	{
		// 1. �v���C���[���C�g���J�����̈ʒu��
		m_lights[0].Position = m_Camera->GetPosition();

		// 2. �p�g���[�����C�g�𓮂��� (�V���v���ȉ~�^��)
		static float totalTime = 0;
		totalTime += deltaTime;
		m_lights[1].Position.x = sin(totalTime) * 15.0f + 15.0f;
		m_lights[1].Position.z = cos(totalTime) * 15.0f + 15.0f;
		m_lights[1].Position.y = 2.0f;

		// 3. �X�|�b�g���C�g��_�ł�����
		m_flickerTimer += deltaTime;
		if (m_flickerTimer > 0.1f) // 0.1�b���Ƃɏ�Ԃ��X�V
		{
			m_flickerTimer = 0.0f;
			if ((rand() % 100) < 30) // 30%�̊m���ŏ�Ԃ�؂�ւ���
			{
				m_lights[2].Enabled = !m_lights[2].Enabled;
			}
		}
	}
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
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
	m_D3D->TurnZBufferOn();

	m_D3D->SetViewMatrix(m_Camera->GetViewMatrix());

	ID3D11DeviceContext* deviceContext = m_D3D->GetDeviceContext();

	m_D3D->UpdateLightBuffer(m_lights.data(), static_cast<int>(m_lights.size()), m_Camera->GetPosition());

	deviceContext->IASetInputLayout(m_D3D->GetInputLayout());
	deviceContext->VSSetShader(m_D3D->GetVertexShader(), nullptr, 0);
	deviceContext->PSSetShader(m_D3D->GetPixelShader(), nullptr, 0);

	ID3D11SamplerState* samplerState = m_D3D->GetSamplerState();
	deviceContext->PSSetSamplers(0, 1, &samplerState);

	// �ǃ��f����`��
	if (m_wallModel)
	{
		// �ǃ��f���͌��_�ɔz�u����Ă���̂ŁA���[���h�s��͒P�ʍs��ŗǂ�
		m_D3D->SetWorldMatrix(DirectX::XMMatrixIdentity());
		m_D3D->UpdateMatrixBuffer();
		m_wallModel->Render(deviceContext);
	}

	// ����`��
	if (m_floorModel)
	{
		XMMATRIX worldMatrix = m_floorModel->GetWorldMatrix();
		m_D3D->SetWorldMatrix(worldMatrix);
		m_D3D->UpdateMatrixBuffer();
		m_floorModel->Render(deviceContext);
	}

	m_D3D->EndScene();
}