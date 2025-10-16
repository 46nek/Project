// GameScene.cpp (この内容で完全に置き換えてください)

#include "GameScene.h"
#include "AssetLoader.h"
#include "Game.h"
#include <random>
#include <tuple>
#include <algorithm> // std::shuffle用

// コンストラクタで収集済みオーブの数を初期化
GameScene::GameScene() : m_collectedOrbsCount(0)
{
}

GameScene::~GameScene() {}

bool GameScene::Initialize(GraphicsDevice* graphicsDevice, Input* input, DirectX::AudioEngine* audioEngine)
{
	m_graphicsDevice = graphicsDevice;
	m_input = input;
	m_audioEngine = audioEngine;

	// ステージの初期化
	m_stage = std::make_unique<Stage>();
	if (!m_stage->Initialize(graphicsDevice))
	{
		return false;
	}

	// その他オブジェクトの初期化
	m_player = std::make_unique<Player>();
	m_camera = std::make_unique<Camera>();
	m_lightManager = std::make_unique<LightManager>();
	m_lightManager->Initialize(m_stage->GetMazeData(), m_stage->GetPathWidth(), Stage::WALL_HEIGHT);
	m_renderer = std::make_unique<Renderer>(m_graphicsDevice);

	// スタート座標をStageから取得してプレイヤーを配置
	std::pair<int, int> startPos = m_stage->GetStartPosition();
	float pathWidth = m_stage->GetPathWidth();
	float startX = (static_cast<float>(startPos.first) + 0.5f) * pathWidth;
	float startZ = (static_cast<float>(startPos.second) + 0.5f) * pathWidth;
	m_player->Initialize({ startX, PLAYER_HEIGHT, startZ });

	// ミニマップにStageの情報を渡して初期化
	m_minimap = std::make_unique<Minimap>();
	if (!m_minimap->Initialize(graphicsDevice, m_stage->GetMazeData(), pathWidth))
	{
		return false;
	}

	// ヘルパー関数を呼び出す
	if (!InitializeEnemies())
	{
		return false;
	}
	if (!InitializeOrbs())
	{
		return false;
	}
	try
	{
		m_collectSound = std::make_unique<DirectX::SoundEffect>(m_audioEngine, L"Assets/orb_get.wav");
	}
	catch (const std::exception& e)
	{
		// ファイルが見つからない場合などのエラー処理
		MessageBoxA(nullptr, e.what(), "Sound Error", MB_OK);
		return false;
	}

	m_uiOrb = AssetLoader::LoadModelFromFile(m_graphicsDevice->GetDevice(), "Assets/cube.obj");
	if (!m_uiOrb)
	{
		return false;
	}
	m_uiOrb->SetScale(0.3f, 0.3f, 0.3f);
	m_uiOrb->SetEmissiveColor({ 0.6f, 0.8f, 1.0f, 1.0f });
	m_uiOrb->SetUseTexture(false);
	m_uiOrb->SetUseNormalMap(false);

	// UI用カメラの初期化
	m_uiCamera = std::make_unique<Camera>();
	m_uiCamera->SetPosition(0.0f, 0.5f, -2.0f);   // 少し上に移動
	m_uiCamera->SetRotation(-15.0f, 0.0f, 0.0f); // 少し下を向かせる

	return true;
}

bool GameScene::InitializeEnemies()
{
	float pathWidth = m_stage->GetPathWidth();

	// 敵の出現候補となる部屋のリスト
	std::vector<std::pair<int, int>> spawnRooms = {
		{1, 1}, // 左上
		{Stage::MAZE_WIDTH - 4, 1}, // 右上
		{1, Stage::MAZE_HEIGHT - 4}, // 左下
		{Stage::MAZE_WIDTH - 4, Stage::MAZE_HEIGHT - 4}  // 右下
	};

	// 部屋のリストをシャッフルして、重複しないように選ぶ
	std::random_device rd;
	std::mt19937 gen(rd());
	std::shuffle(spawnRooms.begin(), spawnRooms.end(), gen);

	// 定数で指定された数の敵を生成
	for (int i = 0; i < NUM_ENEMIES; ++i)
	{
		std::pair<int, int> room = spawnRooms[i];
		float enemyStartX = (static_cast<float>(room.first) + 1.5f) * pathWidth;
		float enemyStartZ = (static_cast<float>(room.second) + 1.5f) * pathWidth;

		auto enemy = std::make_unique<Enemy>();
		if (!enemy->Initialize(m_graphicsDevice->GetDevice(), { enemyStartX, 1.0f, enemyStartZ }, m_stage->GetMazeData()))
		{
			return false;
		}
		m_enemies.push_back(std::move(enemy));
	}
	return true;
}

bool GameScene::InitializeOrbs()
{
	const auto& mazeData = m_stage->GetMazeData();
	float pathWidth = m_stage->GetPathWidth();
	std::vector<std::pair<int, int>> possibleSpawns;

	std::random_device rd;
	std::mt19937 gen(rd());

	// 1. 部屋の範囲を定義 (MazeGenerator.cpp と同じ定義)
	const int roomSize = 3;
	const int cornerOffset = 1;
	using Rect = std::tuple<int, int, int, int>;
	const std::vector<Rect> rooms = {
		std::make_tuple(cornerOffset, cornerOffset, roomSize, roomSize), // 左上
		std::make_tuple(Stage::MAZE_WIDTH - cornerOffset - roomSize, cornerOffset, roomSize, roomSize), // 右上
		std::make_tuple(cornerOffset, Stage::MAZE_HEIGHT - cornerOffset - roomSize, roomSize, roomSize), // 左下
		std::make_tuple(Stage::MAZE_WIDTH - cornerOffset - roomSize, Stage::MAZE_HEIGHT - cornerOffset - roomSize, roomSize, roomSize), // 右下
		std::make_tuple((Stage::MAZE_WIDTH - roomSize) / 2, (Stage::MAZE_HEIGHT - roomSize) / 2, roomSize, roomSize) // 中央
	};

	// 2. オーブの配置候補地（行き止まりや角）を探す
	for (size_t y = 1; y < mazeData.size() - 1; ++y)
	{
		for (size_t x = 1; x < mazeData[0].size() - 1; ++x)
		{
			if (mazeData[y][x] == MazeGenerator::Path)
			{
				bool isInRoom = false;
				for (const auto& r : rooms) {
					int sx = std::get<0>(r), sy = std::get<1>(r), w = std::get<2>(r), h = std::get<3>(r);
					if (x >= sx && x < sx + w && y >= sy && y < sy + h) {
						isInRoom = true;
						break;
					}
				}
				if (isInRoom) continue;

				int pathNeighbors = 0;
				if (mazeData[y - 1][x] == MazeGenerator::Path) pathNeighbors++;
				if (mazeData[y + 1][x] == MazeGenerator::Path) pathNeighbors++;
				if (mazeData[y][x - 1] == MazeGenerator::Path) pathNeighbors++;
				if (mazeData[y][x + 1] == MazeGenerator::Path) pathNeighbors++;

				if (pathNeighbors <= 2)
				{
					possibleSpawns.push_back({ static_cast<int>(x), static_cast<int>(y) });
				}
			}
		}
	}

	std::shuffle(possibleSpawns.begin(), possibleSpawns.end(), gen);

	std::vector<std::pair<int, int>> spawnedOrbPositions;

	// 3. 候補地リストを調べてオーブを配置
	for (const auto& spawnPos : possibleSpawns)
	{
		if (m_orbs.size() >= NUM_ORBS) break;

		bool canSpawn = true;
		for (const auto& spawnedPos : spawnedOrbPositions)
		{
			if (std::abs(spawnPos.first - spawnedPos.first) + std::abs(spawnPos.second - spawnedPos.second) <= 2)
			{
				canSpawn = false;
				break;
			}
		}

		if (canSpawn)
		{
			float orbX = (static_cast<float>(spawnPos.first) + 0.5f) * pathWidth;
			float orbZ = (static_cast<float>(spawnPos.second) + 0.5f) * pathWidth;
			DirectX::XMFLOAT3 orbPos = { orbX, 2.0f, orbZ };

			DirectX::XMFLOAT4 orbColor = { 0.8f, 0.8f, 1.0f, 1.0f };
			float orbRange = 5.0f;
			float orbIntensity = 1.0f;
			int lightIndex = m_lightManager->AddPointLight(orbPos, orbColor, orbRange, orbIntensity);

			if (lightIndex != -1)
			{
				auto orb = std::make_unique<Orb>();
				if (orb->Initialize(m_graphicsDevice->GetDevice(), orbPos, lightIndex))
				{
					m_orbs.push_back(std::move(orb));
					spawnedOrbPositions.push_back(spawnPos);
				}
			}
		}
	}
	return true;
}

void GameScene::Shutdown()
{
	for (auto& orb : m_orbs)
	{
		if (orb) orb->Shutdown();
	}
	m_orbs.clear();

	for (auto& enemy : m_enemies)
	{
		if (enemy) enemy->Shutdown();
	}
	m_enemies.clear();

	if (m_minimap) m_minimap->Shutdown();
	if (m_stage) m_stage->Shutdown();
}

void GameScene::Update(float deltaTime)
{
	int mouseX, mouseY;
	m_input->GetMouseDelta(mouseX, mouseY);
	m_player->Turn(mouseX, mouseY, deltaTime);

	m_player->Update(deltaTime, m_input, m_stage->GetMazeData(), m_stage->GetPathWidth());

	for (auto& enemy : m_enemies)
	{
		enemy->Update(deltaTime, m_player.get(), m_stage->GetMazeData(), m_stage->GetPathWidth());
	}
	for (auto& orb : m_orbs)
	{
		orb->Update(deltaTime, m_player.get(), m_lightManager.get(), m_collectSound.get());
	}

	DirectX::XMFLOAT3 playerPos = m_player->GetPosition();
	DirectX::XMFLOAT3 playerRot = m_player->GetRotation();
	m_camera->SetPosition(playerPos.x, playerPos.y, playerPos.z);
	m_camera->SetRotation(playerRot.x, playerRot.y, playerRot.z);
	if (m_player->IsRunning())
	{
		m_camera->SetBobbingParameters(18.0f, 0.05f, 10.0f, 0.08f, 9.0f, 0.15f);
	}
	else
	{
		m_camera->SetBobbingParameters(14.0f, 0.03f, 7.0f, 0.05f, 7.0f, 0.1f);
	}
	m_camera->UpdateBobbing(deltaTime, m_player->IsMoving());
	m_camera->Update();

	m_lightManager->Update(deltaTime, m_camera->GetPosition(), m_camera->GetRotation());

	// UI用Orbを回転させる
	static float uiOrbRotationY = 0.0f;
	static float uiOrbRotationX = 0.0f;
	uiOrbRotationY += deltaTime * 1.5f;
	uiOrbRotationX += deltaTime * 0.5f; // X軸の回転を追加
	m_uiOrb->SetRotation(uiOrbRotationX, uiOrbRotationY, 0.0f);

	// 収集したオーブの数を更新
	m_collectedOrbsCount = 0;
	for (const auto& orb : m_orbs)
	{
		if (orb->IsCollected())
		{
			m_collectedOrbsCount++;
		}
	}
}

void GameScene::Render()
{
	std::vector<Model*> modelsToRender;
	for (const auto& model : m_stage->GetModels())
	{
		modelsToRender.push_back(model.get());
	}
	for (const auto& enemy : m_enemies)
	{
		modelsToRender.push_back(enemy->GetModel());
	}
	for (const auto& orb : m_orbs)
	{
		if (Model* orbModel = orb->GetModel())
		{
			modelsToRender.push_back(orbModel);
		}
	}

	// 1. 3Dシーンをテクスチャにレンダリング
	m_renderer->RenderSceneToTexture(modelsToRender, m_camera.get(), m_lightManager.get());
	// 2. ポストエフェクトを適用して画面に描画
	m_renderer->RenderFinalPass(m_camera.get());
	// 3. ミニマップを描画
	m_minimap->Render(m_camera.get(), m_enemies, m_orbs);

	ID3D11DeviceContext* deviceContext = m_graphicsDevice->GetDeviceContext();
	ShaderManager* shaderManager = m_graphicsDevice->GetShaderManager();

	// --- UI用Orbの描画 ---
	// a. UI用のビューポートを定義
	D3D11_VIEWPORT uiViewport = {};
	uiViewport.Width = 150.0f;
	uiViewport.Height = 150.0f;
	uiViewport.TopLeftX = Game::SCREEN_WIDTH - uiViewport.Width - 20.0f;
	uiViewport.TopLeftY = 20.0f;
	uiViewport.MinDepth = 0.0f;
	uiViewport.MaxDepth = 1.0f;
	deviceContext->RSSetViewports(1, &uiViewport);

	// b. ★★★★★ 描画状態の完全リセット ★★★★★
	// SpriteBatchなどによる影響を完全に排除するため、関連する全ての描画状態をリセットします。
	m_graphicsDevice->GetSwapChain()->TurnZBufferOff(deviceContext); // 深度テストはOFF
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	deviceContext->OMSetBlendState(m_graphicsDevice->GetDefaultBlendState(), blendFactor, 0xffffffff); // ブレンドをデフォルトに
	deviceContext->RSSetState(m_graphicsDevice->GetDefaultRasterizerState()); // ラスタライザをデフォルトに
	ID3D11ShaderResourceView* nullSRVs[] = { nullptr, nullptr, nullptr };
	deviceContext->PSSetShaderResources(0, 3, nullSRVs); // 全てのテクスチャを解除
	ID3D11SamplerState* nullSamplers[] = { nullptr, nullptr };
	deviceContext->PSSetSamplers(0, 2, nullSamplers); // 全てのサンプラを解除

	// c. メインの3D描画用シェーダーを設定
	deviceContext->IASetInputLayout(shaderManager->GetInputLayout());
	deviceContext->VSSetShader(shaderManager->GetVertexShader(), nullptr, 0);
	deviceContext->PSSetShader(shaderManager->GetPixelShader(), nullptr, 0);

	// d. UI Orb用に照明を「ライト0個」として設定し、照明計算を無効化
	LightBufferType uiLightBuffer = {};
	uiLightBuffer.NumLights = 0;
	uiLightBuffer.CameraPosition = m_uiCamera->GetPosition();
	m_graphicsDevice->UpdateLightBuffer(uiLightBuffer);

	// e. UI用のマテリアル（自己発光色のみ）を設定
	MaterialBufferType materialBuffer;
	materialBuffer.EmissiveColor = m_uiOrb->GetEmissiveColor();
	materialBuffer.UseTexture = false;
	materialBuffer.UseNormalMap = false;
	m_graphicsDevice->UpdateMaterialBuffer(materialBuffer);

	// f. UI用のカメラと行列でモデルを描画
	m_uiCamera->Update();
	DirectX::XMMATRIX uiViewMatrix = m_uiCamera->GetViewMatrix();
	DirectX::XMMATRIX uiProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PI / 4.0f, 1.0f, 0.1f, 100.0f);

	m_graphicsDevice->UpdateMatrixBuffer(
		m_uiOrb->GetWorldMatrix(),
		uiViewMatrix,
		uiProjectionMatrix,
		DirectX::XMMatrixIdentity(),
		DirectX::XMMatrixIdentity()
	);

	// g. Orbを描画
	m_uiOrb->Render(deviceContext);

	// h. 後処理: ビューポートと深度設定をメイン描画用に復元
	D3D11_VIEWPORT mainViewport = {};
	mainViewport.Width = (FLOAT)Game::SCREEN_WIDTH;
	mainViewport.Height = (FLOAT)Game::SCREEN_HEIGHT;
	mainViewport.MaxDepth = 1.0f;
	deviceContext->RSSetViewports(1, &mainViewport);
	m_graphicsDevice->GetSwapChain()->TurnZBufferOn(deviceContext);

	m_graphicsDevice->EndScene();
}