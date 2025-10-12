#include "GameScene.h"
#include "AssetLoader.h"

GameScene::GameScene() {}
GameScene::~GameScene() {}

bool GameScene::Initialize(GraphicsDevice* graphicsDevice, Input* input)
{
    m_graphicsDevice = graphicsDevice;
    m_input = input;

    m_player = std::make_unique<Player>();
    m_camera = std::make_unique<Camera>();
    m_lightManager = std::make_unique<LightManager>();
    m_lightManager->Initialize();

    m_renderer = std::make_unique<Renderer>(m_graphicsDevice);

    m_mazeGenerator = std::make_unique<MazeGenerator>();
    m_mazeGenerator->Generate(MAZE_WIDTH, MAZE_HEIGHT);

    // �v���C���[����H�̓����ɏ�����
    std::pair<int, int> startPos = m_mazeGenerator->GetStartPosition();
    float startX = (static_cast<float>(startPos.first) + 0.5f) * PATH_WIDTH;
    float startZ = (static_cast<float>(startPos.second) + 0.5f) * PATH_WIDTH;
    m_player->Initialize({ startX, PLAYER_HEIGHT, startZ });

    m_minimap = std::make_unique<Minimap>();
    if (!m_minimap->Initialize(graphicsDevice, m_mazeGenerator->GetMazeData()))
    {
        return false;
    }

    // �ǃ��f���̐����ƃe�N�X�`���ݒ�
    auto wallModel = AssetLoader::CreateMazeModel(m_graphicsDevice->GetDevice(), m_mazeGenerator->GetMazeData(), PATH_WIDTH, WALL_HEIGHT, MeshGenerator::MeshType::Wall);
    if (!wallModel) return false;
    wallModel->SetTexture(AssetLoader::LoadTexture(m_graphicsDevice->GetDevice(), L"Assets/wall.png"));
    m_models.push_back(std::move(wallModel));

    // �V�䃂�f���̐����ƃe�N�X�`���ݒ�
    auto ceilingModel = AssetLoader::CreateMazeModel(m_graphicsDevice->GetDevice(), m_mazeGenerator->GetMazeData(), PATH_WIDTH, WALL_HEIGHT, MeshGenerator::MeshType::Ceiling);
    if (!ceilingModel) return false;
    ceilingModel->SetTexture(AssetLoader::LoadTexture(m_graphicsDevice->GetDevice(), L"Assets/wall.png"));
    m_models.push_back(std::move(ceilingModel));

    // �����f���̐����ƃe�N�X�`���ݒ� 
    auto floorModel = AssetLoader::CreateMazeModel(m_graphicsDevice->GetDevice(), m_mazeGenerator->GetMazeData(), PATH_WIDTH, WALL_HEIGHT, MeshGenerator::MeshType::Floor);
    if (!floorModel) return false;
    floorModel->SetTexture(AssetLoader::LoadTexture(m_graphicsDevice->GetDevice(), L"Assets/wall.png"));

    m_models.push_back(std::move(floorModel));

    return true;
}

void GameScene::Shutdown()
{
    if (m_minimap) m_minimap->Shutdown();
    for (auto& model : m_models) {
        if (model) model->Shutdown();
    }
    m_models.clear();
}

void GameScene::Update(float deltaTime)
{
    // �v���C���[�̉�]�i�}�E�X�j
    int mouseX, mouseY;
    m_input->GetMouseDelta(mouseX, mouseY);
    m_player->Turn(mouseX, mouseY, deltaTime);

    // �v���C���[�̈ړ��i�L�[�{�[�h�j�Ɠ����蔻��
    m_player->Update(deltaTime, m_input, m_mazeGenerator->GetMazeData(), PATH_WIDTH);

    // �J�������v���C���[�ɒǏ]
    DirectX::XMFLOAT3 playerPos = m_player->GetPosition();
    DirectX::XMFLOAT3 playerRot = m_player->GetRotation();
    m_camera->SetPosition(playerPos.x, playerPos.y, playerPos.z);
    m_camera->SetRotation(playerRot.x, playerRot.y, playerRot.z);
    
    // �J�����̗h����X�V
    m_camera->UpdateBobbing(deltaTime, m_player->IsMoving());

    m_camera->Update();

    m_lightManager->Update(deltaTime, m_camera->GetPosition(), m_camera->GetRotation());
}

void GameScene::Render()
{
    // �`��J�n
    m_graphicsDevice->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // 3D�V�[���̕`��
    m_renderer->RenderScene(m_models, m_camera.get(), m_lightManager.get());

    // 2D�i�~�j�}�b�v�j�̕`��
    m_minimap->Render(m_camera.get());

    // �`��I��
    m_graphicsDevice->EndScene();
}