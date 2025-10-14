#include "GameScene.h"
#include "AssetLoader.h"

GameScene::GameScene() {}
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

    // 敵を初期化
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

    // 当たり判定にStageの情報を利用
    m_player->Update(deltaTime, m_input, m_stage->GetMazeData(), m_stage->GetPathWidth());

    // 敵を更新
    m_enemy->Update(deltaTime, m_player.get(), m_stage->GetMazeData(), m_stage->GetPathWidth());

    // カメラとライトの更新
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
    // 描画するモデルのリストを作成
    std::vector<Model*> modelsToRender;
    for (const auto& model : m_stage->GetModels())
    {
        modelsToRender.push_back(model.get());
    }
    modelsToRender.push_back(m_enemy->GetModel());

    // モデルリストをレンダラーに渡す
    m_renderer->RenderSceneToTexture(modelsToRender, m_camera.get(), m_lightManager.get());
    m_renderer->RenderFinalPass(m_camera.get());
    m_minimap->Render(m_camera.get(), m_enemy.get());

    m_graphicsDevice->EndScene();
}