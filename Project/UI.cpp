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

    // �~�j�}�b�v��������
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
    // �eUI�R���|�[�l���g��Update���Ăяo��
    // (�~�j�}�b�v�ɂ͌���Update�����͂���܂���)
}

void UI::Render(const Camera* camera, const std::vector<std::unique_ptr<Enemy>>& enemies, const std::vector<std::unique_ptr<Orb>>& orbs)
{
    // �eUI�R���|�[�l���g��Render���Ăяo��
    m_minimap->Render(camera, enemies, orbs);
}