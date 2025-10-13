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

    std::pair<int, int> startPos = m_mazeGenerator->GetStartPosition();
    float startX = (static_cast<float>(startPos.first) + 0.5f) * PATH_WIDTH;
    float startZ = (static_cast<float>(startPos.second) + 0.5f) * PATH_WIDTH;
    m_player->Initialize({ startX, PLAYER_HEIGHT, startZ });

    m_minimap = std::make_unique<Minimap>();
    if (!m_minimap->Initialize(graphicsDevice, m_mazeGenerator->GetMazeData(), PATH_WIDTH))
    {
        return false;
    }

    // --- �e�N�X�`���̎��O�ǂݍ��� ---
    auto wallTexture = AssetLoader::LoadTexture(m_graphicsDevice->GetDevice(), L"Assets/wall.png");
    if (!wallTexture) {
        MessageBox(nullptr, L"Failed to load Assets/wall.png", L"Error", MB_OK);
        return false;
    }
    auto wallNormalMap = AssetLoader::LoadTexture(m_graphicsDevice->GetDevice(), L"Assets/wall_normal.png");
    if (!wallNormalMap) {
        MessageBox(nullptr, L"Failed to load Assets/wall_normal.png.\nMake sure the file exists in the Assets folder.", L"Error", MB_OK);
        return false;
    }
    auto ceilingTexture = AssetLoader::LoadTexture(m_graphicsDevice->GetDevice(), L"Assets/wall.png");
    if (!ceilingTexture) {
        MessageBox(nullptr, L"Failed to load Assets/ceiling.png", L"Error", MB_OK);
        return false;
    }
    auto ceilingNormalMap = AssetLoader::LoadTexture(m_graphicsDevice->GetDevice(), L"Assets/wall_normal.png");
    if (!ceilingNormalMap) {
        MessageBox(nullptr, L"Failed to load Assets/ceiling_normal.png.\nMake sure the file exists in the Assets folder.", L"Error", MB_OK);
        return false;
    }
    auto floorTexture = AssetLoader::LoadTexture(m_graphicsDevice->GetDevice(), L"Assets/wall.png");
    if (!floorTexture) {
        MessageBox(nullptr, L"Failed to load Assets/floor.png", L"Error", MB_OK);
        return false;
    }
    auto floorNormalMap = AssetLoader::LoadTexture(m_graphicsDevice->GetDevice(), L"Assets/wall_normal.png");
    if (!floorNormalMap) {
        MessageBox(nullptr, L"Failed to load Assets/floor_normal.png.\nMake sure the file exists in the Assets folder.", L"Error", MB_OK);
        return false;
    }


    // --- ���f���̐����ƃe�N�X�`���ݒ� ---

    // �ǃ��f���i1�i�ځj
    auto wallModel1 = AssetLoader::CreateMazeModel(m_graphicsDevice->GetDevice(), m_mazeGenerator->GetMazeData(), PATH_WIDTH, WALL_HEIGHT / 2.0f, MeshGenerator::MeshType::Wall);
    if (!wallModel1) {
        MessageBox(nullptr, L"Failed to create wall model 1.", L"Error", MB_OK);
        return false;
    }
    wallModel1->SetTexture(std::move(wallTexture));
    wallModel1->SetNormalMap(std::move(wallNormalMap));
    m_models.push_back(std::move(wallModel1));

    // �ǃ��f���i2�i�ځj - �e�N�X�`�����ēǂݍ���
    wallTexture = AssetLoader::LoadTexture(m_graphicsDevice->GetDevice(), L"Assets/wall.png");
    wallNormalMap = AssetLoader::LoadTexture(m_graphicsDevice->GetDevice(), L"Assets/wall_normal.png");
    auto wallModel2 = AssetLoader::CreateMazeModel(m_graphicsDevice->GetDevice(), m_mazeGenerator->GetMazeData(), PATH_WIDTH, WALL_HEIGHT / 2.0f, MeshGenerator::MeshType::Wall);
    if (!wallModel2) {
        MessageBox(nullptr, L"Failed to create wall model 2.", L"Error", MB_OK);
        return false;
    }
    wallModel2->SetTexture(std::move(wallTexture));
    wallModel2->SetNormalMap(std::move(wallNormalMap));
    wallModel2->SetPosition(0.0f, WALL_HEIGHT / 2.0f, 0.0f);
    m_models.push_back(std::move(wallModel2));

    // �V�䃂�f��
    auto ceilingModel = AssetLoader::CreateMazeModel(m_graphicsDevice->GetDevice(), m_mazeGenerator->GetMazeData(), PATH_WIDTH, WALL_HEIGHT, MeshGenerator::MeshType::Ceiling);
    if (!ceilingModel) {
        MessageBox(nullptr, L"Failed to create ceiling model.", L"Error", MB_OK);
        return false;
    }
    ceilingModel->SetTexture(std::move(ceilingTexture));
    ceilingModel->SetNormalMap(std::move(ceilingNormalMap)); // <--- �m�[�}���}�b�v��ݒ�
    m_models.push_back(std::move(ceilingModel));

    // �����f��
    auto floorModel = AssetLoader::CreateMazeModel(m_graphicsDevice->GetDevice(), m_mazeGenerator->GetMazeData(), PATH_WIDTH, WALL_HEIGHT, MeshGenerator::MeshType::Floor);
    if (!floorModel) {
        MessageBox(nullptr, L"Failed to create floor model.", L"Error", MB_OK);
        return false;
    }
    floorModel->SetTexture(std::move(floorTexture));
    floorModel->SetNormalMap(std::move(floorNormalMap)); // <--- �m�[�}���}�b�v��ݒ�
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
    // --- �V�����`��t���[ ---
    // 1. 3D�V�[���i���H�Ȃǁj���I�t�X�N���[���̃e�N�X�`���ɕ`��
    m_renderer->RenderSceneToTexture(m_models, m_camera.get(), m_lightManager.get());
    
    // 2. �菇1�ŕ`�悵���e�N�X�`������ʂɕ\���i���̒i�K�Ń|�X�g�v���Z�X��K�p�j
    m_renderer->RenderFinalPass(m_camera.get());

    // 2D�i�~�j�}�b�v�j�̕`��
    m_minimap->Render(m_camera.get());

    // �`��I��
    m_graphicsDevice->EndScene();
}