#include "UI.h"
#include "Camera.h" // <<< 前方宣言に対応してインクルード
#include "Enemy.h"  // <<< 前方宣言に対応してインクルード
#include "Orb.h"    // <<< 前方宣言に対応してインクルード

UI::UI() : m_graphicsDevice(nullptr)
{
}

UI::~UI()
{
}

bool UI::Initialize(GraphicsDevice* graphicsDevice, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth)
{
    m_graphicsDevice = graphicsDevice;

    // オーブ表示用のUIコンポーネントを初期化
    m_orbDisplay = std::make_unique<UIOrb>();
    if (!m_orbDisplay->Initialize(m_graphicsDevice))
    {
        return false;
    }

    // ミニマップを初期化
    m_minimap = std::make_unique<Minimap>();
    if (!m_minimap->Initialize(graphicsDevice, mazeData, pathWidth))
    {
        return false;
    }

    return true;
}

void UI::Shutdown()
{
    if (m_orbDisplay)
    {
        m_orbDisplay->Shutdown();
    }
    if (m_minimap)
    {
        m_minimap->Shutdown();
    }
}

void UI::Update(float deltaTime, int collectedOrbsCount)
{
    // 各UIコンポーネントのUpdateを呼び出す
    m_orbDisplay->Update(deltaTime);
    // m_minimapはUpdate不要
}

void UI::Render(const Camera* camera, const std::vector<std::unique_ptr<Enemy>>& enemies, const std::vector<std::unique_ptr<Orb>>& orbs)
{
    // 各UIコンポーネントのRenderを呼び出す
    m_minimap->Render(camera, enemies, orbs);
    m_orbDisplay->Render();
}