#include "Stage.h"
#include "AssetLoader.h"
#include <windows.h>

Stage::Stage() {}
Stage::~Stage() {}

bool Stage::Initialize(GraphicsDevice* graphicsDevice)
{
	m_mazeGenerator = std::make_unique<MazeGenerator>();
	m_mazeGenerator->Generate(MAZE_WIDTH, MAZE_HEIGHT);
	
	m_exitX = (MAZE_WIDTH - 1) / 2;
	m_exitY = 0;

	m_mazeGenerator->SetCell(m_exitX, m_exitY, MazeGenerator::Path);
	
	// AssetLoader::LoadTexture は unique_ptr を返しますが、shared_ptr に代入することで所有権を共有可能にします
	std::shared_ptr<Texture> wallTexture = AssetLoader::LoadTexture(graphicsDevice->GetDevice(), L"Assets/wall.png");
	if (!wallTexture) { MessageBox(nullptr, L"Failed to load wall.png", L"Error", MB_OK); return false; }

	std::shared_ptr<Texture> wallNormalMap = AssetLoader::LoadTexture(graphicsDevice->GetDevice(), L"Assets/wall_normal.png");
	if (!wallNormalMap) { MessageBox(nullptr, L"Failed to load wall_normal.png", L"Error", MB_OK); return false; }

	// モデルの生成
	auto wallModel1 = AssetLoader::CreateMazeModel(graphicsDevice->GetDevice(), m_mazeGenerator->GetMazeData(), PATH_WIDTH, WALL_HEIGHT / 2.0f, MeshGenerator::MeshType::Wall);
	if (!wallModel1) return false;
	wallModel1->SetTexture(wallTexture);
	wallModel1->SetNormalMap(wallNormalMap);
	m_models.push_back(std::move(wallModel1));

	// 2段目の壁
	auto wallModel2 = AssetLoader::CreateMazeModel(graphicsDevice->GetDevice(), m_mazeGenerator->GetMazeData(), PATH_WIDTH, WALL_HEIGHT / 2.0f, MeshGenerator::MeshType::Wall);
	if (!wallModel2) return false;
	wallModel2->SetTexture(wallTexture);      // 共通テクスチャ
	wallModel2->SetNormalMap(wallNormalMap);  // 共通ノーマルマップ
	wallModel2->SetPosition(0.0f, WALL_HEIGHT / 2.0f, 0.0f);
	m_models.push_back(std::move(wallModel2));

	// 天井
	auto ceilingModel = AssetLoader::CreateMazeModel(graphicsDevice->GetDevice(), m_mazeGenerator->GetMazeData(), PATH_WIDTH, WALL_HEIGHT, MeshGenerator::MeshType::Ceiling);
	if (!ceilingModel) return false;
	ceilingModel->SetTexture(wallTexture);      // 共通テクスチャ
	ceilingModel->SetNormalMap(wallNormalMap);  // 共通ノーマルマップ
	m_models.push_back(std::move(ceilingModel));

	// 床 
	auto floorModel = AssetLoader::CreateMazeModel(graphicsDevice->GetDevice(), m_mazeGenerator->GetMazeData(), PATH_WIDTH, WALL_HEIGHT, MeshGenerator::MeshType::Floor);
	if (!floorModel) return false;
	floorModel->SetTexture(wallTexture);      // 共通テクスチャ
	floorModel->SetNormalMap(wallNormalMap);  // 共通ノーマルマップ
	m_models.push_back(std::move(floorModel));

	m_mazeGenerator->SetCell(m_exitX, m_exitY, MazeGenerator::Wall);

	m_gateModel = AssetLoader::LoadModelFromFile(graphicsDevice->GetDevice(), "Assets/cube.fbx"); // cube.objでも可
	if (m_gateModel)
	{
		float gateX = (static_cast<float>(m_exitX) + 0.5f) * PATH_WIDTH;
		float gateZ = (static_cast<float>(m_exitY) + 0.5f) * PATH_WIDTH;

		m_gateModel->SetScale(PATH_WIDTH, WALL_HEIGHT, PATH_WIDTH);
		m_gateModel->SetPosition(gateX, WALL_HEIGHT / 2.0f, gateZ);
		m_gateModel->SetTexture(wallTexture);
		m_gateModel->SetNormalMap(wallNormalMap);
	}

	return true;
}

void Stage::OpenExit()
{
	// 迷路データ上で出口を「道」にする（プレイヤーが通れるようになる）
	m_mazeGenerator->SetCell(m_exitX, m_exitY, MazeGenerator::Path);

	// ゲートモデルを消す（nullptrにして描画・更新しないようにする）
	if (m_gateModel)
	{
		m_gateModel->Shutdown();
		m_gateModel.reset();
	}
}

Model* Stage::GetGateModel() const
{
	return m_gateModel.get();
}

void Stage::Shutdown()
{
	for (auto& model : m_models) {
		if (model) model->Shutdown();
	}
	m_models.clear(); 
	if (m_gateModel) m_gateModel->Shutdown();
}

const std::vector<std::unique_ptr<Model>>& Stage::GetModels() const { return m_models; }
const std::vector<std::vector<MazeGenerator::CellType>>& Stage::GetMazeData() const { return m_mazeGenerator->GetMazeData(); }
std::pair<int, int> Stage::GetStartPosition() const { return m_mazeGenerator->GetStartPosition(); }
float Stage::GetPathWidth() const { return PATH_WIDTH; }