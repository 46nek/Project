#include "Stage.h"
#include "AssetLoader.h"
#include <windows.h> // MessageBox用

Stage::Stage() {}
Stage::~Stage() {}

bool Stage::Initialize(GraphicsDevice* graphicsDevice)
{
    m_mazeGenerator = std::make_unique<MazeGenerator>();
    m_mazeGenerator->Generate(MAZE_WIDTH, MAZE_HEIGHT);

    // テクスチャの読み込み
    auto wallTexture = AssetLoader::LoadTexture(graphicsDevice->GetDevice(), L"Assets/wall.png");
    if (!wallTexture) { MessageBox(nullptr, L"Failed to load wall.png", L"Error", MB_OK); return false; }

    auto wallNormalMap = AssetLoader::LoadTexture(graphicsDevice->GetDevice(), L"Assets/wall_normal.png");
    if (!wallNormalMap) { MessageBox(nullptr, L"Failed to load wall_normal.png", L"Error", MB_OK); return false; }

    // 天井・床用のテクスチャも同様に読み込む
    auto ceilingTexture = AssetLoader::LoadTexture(graphicsDevice->GetDevice(), L"Assets/wall.png");
    if (!ceilingTexture) { MessageBox(nullptr, L"Failed to load ceiling.png", L"Error", MB_OK); return false; }
    auto ceilingNormalMap = AssetLoader::LoadTexture(graphicsDevice->GetDevice(), L"Assets/wall_normal.png");
    if (!ceilingNormalMap) { MessageBox(nullptr, L"Failed to load ceiling_normal.png", L"Error", MB_OK); return false; }

    auto floorTexture = AssetLoader::LoadTexture(graphicsDevice->GetDevice(), L"Assets/wall.png");
    if (!floorTexture) { MessageBox(nullptr, L"Failed to load floor.png", L"Error", MB_OK); return false; }
    auto floorNormalMap = AssetLoader::LoadTexture(graphicsDevice->GetDevice(), L"Assets/wall_normal.png");
    if (!floorNormalMap) { MessageBox(nullptr, L"Failed to load floor_normal.png", L"Error", MB_OK); return false; }


    // モデルの生成
    auto wallModel1 = AssetLoader::CreateMazeModel(graphicsDevice->GetDevice(), m_mazeGenerator->GetMazeData(), PATH_WIDTH, WALL_HEIGHT / 2.0f, MeshGenerator::MeshType::Wall);
    if (!wallModel1) return false;
    wallModel1->SetTexture(std::move(wallTexture));
    wallModel1->SetNormalMap(std::move(wallNormalMap));
    m_models.push_back(std::move(wallModel1));

    // 2段目の壁用にテクスチャを再読み込み
    wallTexture = AssetLoader::LoadTexture(graphicsDevice->GetDevice(), L"Assets/wall.png");
    wallNormalMap = AssetLoader::LoadTexture(graphicsDevice->GetDevice(), L"Assets/wall_normal.png");
    auto wallModel2 = AssetLoader::CreateMazeModel(graphicsDevice->GetDevice(), m_mazeGenerator->GetMazeData(), PATH_WIDTH, WALL_HEIGHT / 2.0f, MeshGenerator::MeshType::Wall);
    if (!wallModel2) return false;
    wallModel2->SetTexture(std::move(wallTexture));
    wallModel2->SetNormalMap(std::move(wallNormalMap));
    wallModel2->SetPosition(0.0f, WALL_HEIGHT / 2.0f, 0.0f);
    m_models.push_back(std::move(wallModel2));

    auto ceilingModel = AssetLoader::CreateMazeModel(graphicsDevice->GetDevice(), m_mazeGenerator->GetMazeData(), PATH_WIDTH, WALL_HEIGHT, MeshGenerator::MeshType::Ceiling);
    if (!ceilingModel) return false;
    ceilingModel->SetTexture(std::move(ceilingTexture));
    ceilingModel->SetNormalMap(std::move(ceilingNormalMap));
    m_models.push_back(std::move(ceilingModel));

    auto floorModel = AssetLoader::CreateMazeModel(graphicsDevice->GetDevice(), m_mazeGenerator->GetMazeData(), PATH_WIDTH, WALL_HEIGHT, MeshGenerator::MeshType::Floor);
    if (!floorModel) return false;
    floorModel->SetTexture(std::move(floorTexture));
    floorModel->SetNormalMap(std::move(floorNormalMap));
    m_models.push_back(std::move(floorModel));

    return true;
}

void Stage::Shutdown()
{
    for (auto& model : m_models) {
        if (model) model->Shutdown();
    }
    m_models.clear();
}

const std::vector<std::unique_ptr<Model>>& Stage::GetModels() const { return m_models; }
const std::vector<std::vector<MazeGenerator::CellType>>& Stage::GetMazeData() const { return m_mazeGenerator->GetMazeData(); }
std::pair<int, int> Stage::GetStartPosition() const { return m_mazeGenerator->GetStartPosition(); }
float Stage::GetPathWidth() const { return PATH_WIDTH; }