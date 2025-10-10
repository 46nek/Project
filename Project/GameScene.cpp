#include "GameScene.h"
#include "AssetLoader.h"

GameScene::GameScene() {}
GameScene::~GameScene() {}

bool GameScene::Initialize(GraphicsDevice* graphicsDevice, Input* input)
{
    m_graphicsDevice = graphicsDevice;
    m_input = input;

    m_camera = std::make_unique<Camera>();
    m_lightManager = std::make_unique<LightManager>();
    m_lightManager->Initialize();

    m_renderer = std::make_unique<Renderer>(m_graphicsDevice);

    m_mazeGenerator = std::make_unique<MazeGenerator>();
    m_mazeGenerator->Generate(21, 21);

    auto wallModel = AssetLoader::CreateWallModelFromMaze(m_graphicsDevice->GetDevice(), m_mazeGenerator->GetMazeData(), 2.0f, 2.0f);
    if (!wallModel) return false;
    wallModel->SetTexture(AssetLoader::LoadTexture(m_graphicsDevice->GetDevice(), L"Assets/test.png")); 
    m_models.push_back(std::move(wallModel));

    auto floorModel = AssetLoader::LoadModelFromFile(m_graphicsDevice->GetDevice(), "Assets/cube.fbx");
    if (!floorModel) return false;
    floorModel->SetTexture(AssetLoader::LoadTexture(m_graphicsDevice->GetDevice(), L"Assets/floor.png"));
    floorModel->SetPosition(20.0f, -1.0f, 20.0f);
    floorModel->SetScale(21.0f, 1.0f, 21.0f);
    m_models.push_back(std::move(floorModel));

    m_camera->SetPosition(2.0f, 1.5f, 2.0f);

    return true;
}

void GameScene::Shutdown()
{
    for (auto& model : m_models) {
        if (model) model->Shutdown();
    }
    m_models.clear();
}

void GameScene::Update(float deltaTime)
{
    HandleInput(deltaTime);
    int mouseX, mouseY;
    m_input->GetMouseDelta(mouseX, mouseY);
    m_camera->Turn(mouseX, mouseY, deltaTime);
    m_camera->Update();
    m_lightManager->Update(deltaTime, m_camera->GetPosition());
}

void GameScene::HandleInput(float deltaTime)
{
    if (m_input->IsKeyDown('W')) m_camera->MoveForward(deltaTime);
    if (m_input->IsKeyDown('S')) m_camera->MoveBackward(deltaTime);
    if (m_input->IsKeyDown('A')) m_camera->MoveLeft(deltaTime);
    if (m_input->IsKeyDown('D')) m_camera->MoveRight(deltaTime);
}

void GameScene::Render()
{
    m_renderer->RenderScene(m_models, m_camera.get(), m_lightManager.get());
}