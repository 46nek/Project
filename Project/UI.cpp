#include "UI.h"
#include "Camera.h" 
#include "Enemy.h"  
#include "Orb.h"    

UI::UI() : m_graphicsDevice(nullptr)
{
}

UI::~UI()
{
}

bool UI::Initialize(GraphicsDevice* graphicsDevice, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth)
{
    m_graphicsDevice = graphicsDevice;

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
    if (m_minimap)
    {
        m_minimap->Shutdown();
    }
}

void UI::Update(float deltaTime)
{
    // 各UIコンポーネントのUpdateを呼び出す
    // (ミニマップには現在Update処理はありません)
}

void UI::Render(const Camera* camera, const std::vector<std::unique_ptr<Enemy>>& enemies, const std::vector<std::unique_ptr<Orb>>& orbs)
{
    // 各UIコンポーネントのRenderを呼び出す
    m_minimap->Render(camera, enemies, orbs);
}