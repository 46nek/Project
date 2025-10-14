#include "GameScene.h"
#include "AssetLoader.h"

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

    // �G��������
    m_enemy = std::make_unique<Enemy>();
    if (!m_enemy->Initialize(m_graphicsDevice->GetDevice(), { startX, 1.0f, startZ }, m_stage->GetMazeData()))
    {
        return false;
    }

    return true;
}

void GameScene::Shutdown()
{
    if (m_enemy) m_enemy->Shutdown();
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

    // �G���X�V
    m_enemy->Update(deltaTime, m_player.get(), m_stage->GetMazeData(), m_stage->GetPathWidth());

    // �J�����ƃ��C�g�̍X�V
    DirectX::XMFLOAT3 playerPos = m_player->GetPosition();
    DirectX::XMFLOAT3 playerRot = m_player->GetRotation();
    m_camera->SetPosition(playerPos.x, playerPos.y, playerPos.z);
    m_camera->SetRotation(playerRot.x, playerRot.y, playerRot.z);
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
    modelsToRender.push_back(m_enemy->GetModel());

    // ���f�����X�g�������_���[�ɓn��
    m_renderer->RenderSceneToTexture(modelsToRender, m_camera.get(), m_lightManager.get());
    m_renderer->RenderFinalPass(m_camera.get());
    m_minimap->Render(m_camera.get(), m_enemy.get());

    m_graphicsDevice->EndScene();
}