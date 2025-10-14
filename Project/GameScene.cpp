// GameScene.cpp

#include "GameScene.h"
#include "AssetLoader.h"
#include <random>

GameScene::GameScene()
{
}
GameScene::~GameScene() {}

bool GameScene::Initialize(GraphicsDevice* graphicsDevice, Input* input)
{
    // ... (�ύX�Ȃ�) ...
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
    const auto& mazeData = m_stage->GetMazeData();
    std::vector<std::pair<int, int>> possibleSpawns;

    // �I�[�u�̔z�u���n�i�s���~�܂��p�j��T��
    for (size_t y = 1; y < mazeData.size() - 1; ++y)
    {
        for (size_t x = 1; x < mazeData[0].size() - 1; ++x)
        {
            if (mazeData[y][x] == MazeGenerator::Path)
            {
                // �אڂ���ʘH�̐��𐔂���
                int pathNeighbors = 0;
                if (mazeData[y - 1][x] == MazeGenerator::Path) pathNeighbors++;
                if (mazeData[y + 1][x] == MazeGenerator::Path) pathNeighbors++;
                if (mazeData[y][x - 1] == MazeGenerator::Path) pathNeighbors++;
                if (mazeData[y][x + 1] == MazeGenerator::Path) pathNeighbors++;

                // �ʘH��2�ȉ��Ȃ���n�Ƃ���
                if (pathNeighbors >= 2)
                {
                    possibleSpawns.push_back({ static_cast<int>(x), static_cast<int>(y) });
                }
            }
        }
    }

    // ���n���V���b�t��
    std::shuffle(possibleSpawns.begin(), possibleSpawns.end(), gen);

    int numOrbs = 50; // ��������I�[�u�̐�
    for (int i = 0; i < numOrbs && i < possibleSpawns.size(); ++i)
    {
        // �I�[�u�̍��W���v�Z
        float orbX = (static_cast<float>(possibleSpawns[i].first) + 0.5f) * pathWidth;
        float orbZ = (static_cast<float>(possibleSpawns[i].second) + 0.5f) * pathWidth;
        DirectX::XMFLOAT3 orbPos = { orbX, 2.0f, orbZ }; // ������������

        // �I�[�u�p�̃|�C���g���C�g���쐬
        DirectX::XMFLOAT4 orbColor = { 0.8f, 0.8f, 1.0f, 1.0f }; // ������
        float orbRange = 5.0f;
        float orbIntensity = 1.0f;
        int lightIndex = m_lightManager->AddPointLight(orbPos, orbColor, orbRange, orbIntensity);

        if (lightIndex != -1)
        {
            // �I�[�u�𐶐����ă��X�g�ɒǉ�
            auto orb = std::make_unique<Orb>();
            if (orb->Initialize(m_graphicsDevice->GetDevice(), orbPos, lightIndex))
            {
                m_orbs.push_back(std::move(orb));
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
    m_minimap->Render(m_camera.get(), m_enemies);

    m_graphicsDevice->EndScene();
}