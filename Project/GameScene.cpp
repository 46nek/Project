// GameScene.cpp (この内容で完全に置き換えてください)

#include "GameScene.h"
#include "AssetLoader.h"
#include <random>
#include <tuple> // std::tupleを使用するために追加

GameScene::GameScene()
{
}
GameScene::~GameScene() {}

bool GameScene::Initialize(GraphicsDevice* graphicsDevice, Input* input)
{
	m_graphicsDevice = graphicsDevice;
	m_input = input;

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
	m_lightManager->Initialize(m_stage->GetMazeData(), m_stage->GetPathWidth(), 5.0f /*WALL_HEIGHT*/);
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

	// 2体の敵を生成
	for (int i = 0; i < 2; ++i)
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
	const auto& mazeData = m_stage->GetMazeData();
	std::vector<std::pair<int, int>> possibleSpawns;

	// ▼▼▼ ここからオーブ生成ロジックの修正 ▼▼▼

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
				// 2a. 部屋の中かどうかをチェック
				bool isInRoom = false;
				for (const auto& r : rooms) {
					int sx = std::get<0>(r), sy = std::get<1>(r), w = std::get<2>(r), h = std::get<3>(r);
					if (x >= sx && x < sx + w && y >= sy && y < sy + h) {
						isInRoom = true;
						break;
					}
				}
				// 部屋の中なら候補地から除外
				if (isInRoom) continue;

				// 2b. 隣接する通路の数を数える
				int pathNeighbors = 0;
				if (mazeData[y - 1][x] == MazeGenerator::Path) pathNeighbors++;
				if (mazeData[y + 1][x] == MazeGenerator::Path) pathNeighbors++;
				if (mazeData[y][x - 1] == MazeGenerator::Path) pathNeighbors++;
				if (mazeData[y][x + 1] == MazeGenerator::Path) pathNeighbors++;

				// 通路が2つ以下（行き止まり or 角）なら候補地とする
				if (pathNeighbors <= 2)
				{
					possibleSpawns.push_back({ static_cast<int>(x), static_cast<int>(y) });
				}
			}
		}
	}

	// 候補地をシャッフル
	std::shuffle(possibleSpawns.begin(), possibleSpawns.end(), gen);

	int numOrbs = 50; // 生成するオーブの数
	std::vector<std::pair<int, int>> spawnedOrbPositions; // 実際にオーブを配置した座標を記録

	// 3. 候補地リストを調べてオーブを配置
	for (const auto& spawnPos : possibleSpawns)
	{
		// 既に配置したい数のオーブを配置済みならループを抜ける
		if (m_orbs.size() >= numOrbs) break;

		bool canSpawn = true;
		// 3a. 既に配置済みのオーブとの距離をチェック
		for (const auto& spawnedPos : spawnedOrbPositions)
		{
			// マンハッタン距離で2マス以内なら配置しない
			if (std::abs(spawnPos.first - spawnedPos.first) + std::abs(spawnPos.second - spawnedPos.second) <= 2)
			{
				canSpawn = false;
				break;
			}
		}

		// 配置可能ならオーブを生成
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
					spawnedOrbPositions.push_back(spawnPos); // 配置した座標を記録
				}
			}
		}
	}
	// ▲▲▲ オーブ生成ロジックの修正ここまで ▲▲▲

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
		orb->Update(deltaTime, m_player.get(), m_lightManager.get());
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

	m_renderer->RenderSceneToTexture(modelsToRender, m_camera.get(), m_lightManager.get());
	m_renderer->RenderFinalPass(m_camera.get());
	m_minimap->Render(m_camera.get(), m_enemies, m_orbs);

	m_graphicsDevice->EndScene();
}