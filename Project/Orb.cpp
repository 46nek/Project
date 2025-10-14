#include "Orb.h"
#include "AssetLoader.h"
#include <cmath> // sin関数用

Orb::Orb()
    : m_isCollected(false),
    m_lightIndex(-1),
    m_animationTimer(0.0f)
{
}

Orb::~Orb()
{
}

bool Orb::Initialize(ID3D11Device* device, const DirectX::XMFLOAT3& position, int lightIndex)
{
    m_position = position;
    m_lightIndex = lightIndex;

    // 敵と同じモデルを流用し、小さくしてオーブに見立てます
    m_model = AssetLoader::LoadModelFromFile(device, "Assets/cube.obj");
    if (!m_model)
    {
        return false;
    }
    m_model->SetScale(0.3f, 0.3f, 0.3f);
    m_model->SetPosition(m_position.x, m_position.y, m_position.z);

    // 自己発光色（青白く輝く色）を設定
    m_model->SetEmissiveColor({ 0.6f, 0.8f, 1.0f, 1.0f });
    // テクスチャは使用しない
    m_model->SetUseTexture(false);

    // アニメーションの開始タイミングをずらすためにランダムな値で初期化
    m_animationTimer = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 10.0f;

    return true;
}

void Orb::Shutdown()
{
    if (m_model)
    {
        m_model->Shutdown();
    }
}

void Orb::Update(float deltaTime, Player* player, LightManager* lightManager)
{
    // 回収済みなら何もしない
    if (m_isCollected)
    {
        return;
    }

    // ふわふわと上下に浮遊するアニメーション
    m_animationTimer += deltaTime;
    float floatingY = m_position.y + sin(m_animationTimer * 2.0f) * 0.25f;
    m_model->SetPosition(m_position.x, floatingY, m_position.z);

    // プレイヤーとの当たり判定
    DirectX::XMFLOAT3 playerPos = player->GetPosition();
    float dx = playerPos.x - m_position.x;
    float dy = playerPos.y - floatingY;
    float dz = playerPos.z - m_position.z;
    float distance = sqrt(dx * dx + dy * dy + dz * dz);

    // プレイヤーとの距離が一定以下になったら回収
    if (distance < 1.0f)
    {
        m_isCollected = true;
        // 対応するライトを無効化する
        if (lightManager && m_lightIndex != -1)
        {
            lightManager->SetLightEnabled(m_lightIndex, false);
        }
    }
}

Model* Orb::GetModel()
{
    // 回収済みの場合はnullptrを返し、描画されないようにする
    return m_isCollected ? nullptr : m_model.get();
}

bool Orb::IsCollected() const
{
    return m_isCollected;
}