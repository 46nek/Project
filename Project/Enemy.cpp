#include "Enemy.h"
#include "AssetLoader.h"
#include <random>

// コンストラクタ
Enemy::Enemy()
    : m_speed(2.0f),
    m_pathIndex(-1),
    m_pathCooldown(0.0f),
    m_currentState(AIState::Wandering), // 初期状態は「徘徊」
    m_stateTimer(0.0f)
{
}

Enemy::~Enemy() {}

// 初期化
bool Enemy::Initialize(ID3D11Device* device, const DirectX::XMFLOAT3& startPosition, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData)
{
    m_position = startPosition;
    m_model = AssetLoader::LoadModelFromFile(device, "Assets/cube.fbx");
    if (!m_model) return false;

    m_model->SetPosition(startPosition.x, startPosition.y, startPosition.z);

    // A*探索用のインスタンスを作成
    m_astar = std::make_unique<AStar>(mazeData);

    return true;
}

void Enemy::Shutdown()
{
    if (m_model) m_model->Shutdown();
}

// プレイヤーが見えるかどうかの判定（簡易版）
bool Enemy::CanSeePlayer(const Player* player, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth)
{
    if (!player) return false;

    DirectX::XMFLOAT3 playerPos = player->GetPosition();
    DirectX::XMVECTOR vecToPlayer = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&playerPos), DirectX::XMLoadFloat3(&m_position));

    // プレイヤーとの距離を計算
    float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(vecToPlayer));

    // 距離が15.0fより遠ければ「見えない」
    if (distance > 15.0f)
    {
        return false;
    }

    // 現状は距離だけで判定（今後のステップで視界や壁の判定を追加）
    return true;
}

// 徘徊モードの処理
void Enemy::Wander(float deltaTime, float pathWidth)
{
    m_stateTimer -= deltaTime;
    // 目標地点に到着したか、タイマーが切れたら新しい目標地点を設定
    if (m_pathIndex == -1 || m_stateTimer <= 0.0f)
    {
        m_stateTimer = 5.0f; // 5秒ごとに新しい場所へ

        int currentX = static_cast<int>(m_position.x / pathWidth);
        int currentY = static_cast<int>(m_position.z / pathWidth);

        // 現在地からランダムな目標地点を決定
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(-5, 5);
        int targetX = currentX + distrib(gen);
        int targetY = currentY + distrib(gen);

        // 経路探索
        m_path = m_astar->FindPath(currentX, currentY, targetX, targetY);
        m_pathIndex = m_path.empty() ? -1 : 1;
    }
}

// 毎フレームの更新処理
void Enemy::Update(float deltaTime, const Player* player, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth)
{
    if (!player || !m_model) return;

    // === 状態遷移の判定 ===
    switch (m_currentState)
    {
    case AIState::Wandering:
        m_speed = 4.0f; // 徘徊時の速度
        // プレイヤーが見えたら追跡モードに切り替え
        if (CanSeePlayer(player, mazeData, pathWidth))
        {
            m_currentState = AIState::Chasing;
            m_pathCooldown = 0.0f; // すぐに経路探索を開始
            // ここで「発見時の叫び声」などのサウンドを再生すると効果的！
        }
        break;

    case AIState::Chasing:
        m_speed = 7.0f; // 追跡時の速度（速くする！）
        // プレイヤーが見えなくなって5秒経ったら徘徊モードに戻る
        if (!CanSeePlayer(player, mazeData, pathWidth))
        {
            m_stateTimer += deltaTime;
            if (m_stateTimer > 5.0f)
            {
                m_currentState = AIState::Wandering;
                m_pathIndex = -1; // パスをリセット
            }
        }
        else
        {
            m_stateTimer = 0.0f; // 見えている間はタイマーをリセット
        }
        break;
    }

    // === 状態ごとの行動 ===
    switch (m_currentState)
    {
    case AIState::Wandering:
        Wander(deltaTime, pathWidth);
        break;

    case AIState::Chasing:
        // 経路探索のクールダウン処理 (1秒ごと)
        m_pathCooldown -= deltaTime;
        if (m_pathCooldown <= 0.0f)
        {
            m_pathCooldown = 1.0f;
            DirectX::XMFLOAT3 playerPos = player->GetPosition();
            int startX = static_cast<int>(m_position.x / pathWidth);
            int startY = static_cast<int>(m_position.z / pathWidth);
            int goalX = static_cast<int>(playerPos.x / pathWidth);
            int goalY = static_cast<int>(playerPos.z / pathWidth);
            m_path = m_astar->FindPath(startX, startY, goalX, goalY);
            m_pathIndex = m_path.empty() ? -1 : 1;
        }
        break;
    }


    // === パスに沿った移動処理 (共通) ===
    if (m_pathIndex != -1 && m_pathIndex < m_path.size())
    {
        DirectX::XMFLOAT2 nextGridPos = m_path[m_pathIndex];
        DirectX::XMFLOAT3 targetPosition = {
            (nextGridPos.x + 0.5f) * pathWidth,
            m_position.y,
            (nextGridPos.y + 0.5f) * pathWidth
        };

        DirectX::XMVECTOR vecToTarget = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&targetPosition), DirectX::XMLoadFloat3(&m_position));
        float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(vecToTarget));

        if (distance < 0.1f)
        {
            m_pathIndex++;
        }
        else
        {
            DirectX::XMVECTOR moveVec = DirectX::XMVector3Normalize(vecToTarget);
            moveVec = DirectX::XMVectorScale(moveVec, m_speed * deltaTime);
            DirectX::XMStoreFloat3(&m_position, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&m_position), moveVec));
            m_model->SetPosition(m_position.x, m_position.y, m_position.z);
            // ここで「足音」を再生すると良い
        }
    }
}

Model* Enemy::GetModel()
{
    return m_model.get();
}

DirectX::XMFLOAT3 Enemy::GetPosition() const
{
    return m_position;
}