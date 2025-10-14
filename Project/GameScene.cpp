#include "GameScene.h"
#include "AssetLoader.h"
#include <random>

GameScene::GameScene() {}
GameScene::~GameScene() {}

bool GameScene::Initialize(GraphicsDevice* graphicsDevice, Input* input)
{
    m_graphicsDevice = graphicsDevice;
    m_input = input;

    // �X�e�[�W�̏�����
    m_stage = std::make_unique<Stage>();
    if (!m_stage->Initialize(graphicsDevice))
    {
        return false;
    }

    // ���̑��I�u�W�F�N�g�̏�����
    m_player = std::make_unique<Player>();
    m_camera = std::make_unique<Camera>();
    m_lightManager = std::make_unique<LightManager>();
    m_lightManager->Initialize(m_stage->GetMazeData(), m_stage->GetPathWidth(), 5.0f /*WALL_HEIGHT*/);
    m_renderer = std::make_unique<Renderer>(m_graphicsDevice);

    // �X�^�[�g���W��Stage����擾���ăv���C���[��z�u
    std::pair<int, int> startPos = m_stage->GetStartPosition();
    float pathWidth = m_stage->GetPathWidth();
    float startX = (static_cast<float>(startPos.first) + 0.5f) * pathWidth;
    float startZ = (static_cast<float>(startPos.second) + 0.5f) * pathWidth;
    m_player->Initialize({ startX, PLAYER_HEIGHT, startZ });

    // �~�j�}�b�v��Stage�̏���n���ď�����
    m_minimap = std::make_unique<Minimap>();
    if (!m_minimap->Initialize(graphicsDevice, m_stage->GetMazeData(), pathWidth))
    {
        return false;
    }

    // �G�̏o�����ƂȂ镔���̃��X�g
    std::vector<std::pair<int, int>> spawnRooms = {
    {1, 1}, // ����
    {Stage::MAZE_WIDTH - 4, 1}, // �E��
    {1, Stage::MAZE_HEIGHT - 4}, // ����
    {Stage::MAZE_WIDTH - 4, Stage::MAZE_HEIGHT - 4}  // �E��
    };

    // �����̃��X�g���V���b�t�����āA�d�����Ȃ��悤�ɑI��
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(spawnRooms.begin(), spawnRooms.end(), gen);
    
            // 2�̂̓G�𐶐�
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

    return true;
}

void GameScene::Shutdown()
{
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

    // �����蔻���Stage�̏��𗘗p
    m_player->Update(deltaTime, m_input, m_stage->GetMazeData(), m_stage->GetPathWidth());

    // ���ׂĂ̓G���X�V
    for (auto& enemy : m_enemies)
    {
        enemy->Update(deltaTime, m_player.get(), m_stage->GetMazeData(), m_stage->GetPathWidth());
    }

    // �J�����ƃ��C�g�̍X�V
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
    m_lightManager->Update(deltaTime, m_camera->GetPosition());
}

void GameScene::Render()
{
    // �`�悷�郂�f���̃��X�g���쐬
    std::vector<Model*> modelsToRender;
    for (const auto& model : m_stage->GetModels())
    {
        modelsToRender.push_back(model.get());
    }
    for (const auto& enemy : m_enemies)
        {
            modelsToRender.push_back(enemy->GetModel());
        }

    // ���f�����X�g�������_���[�ɓn��
    m_renderer->RenderSceneToTexture(modelsToRender, m_camera.get(), m_lightManager.get());
    m_renderer->RenderFinalPass(m_camera.get());
    m_minimap->Render(m_camera.get(), m_enemies);

    m_graphicsDevice->EndScene();
}