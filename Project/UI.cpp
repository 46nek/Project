#include "UI.h"
#include "Camera.h" // <<< �O���錾�ɑΉ����ăC���N���[�h
#include "Enemy.h"  // <<< �O���錾�ɑΉ����ăC���N���[�h
#include "Orb.h"    // <<< �O���錾�ɑΉ����ăC���N���[�h

UI::UI() : m_graphicsDevice(nullptr)
{
}

UI::~UI()
{
}

bool UI::Initialize(GraphicsDevice* graphicsDevice, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth)
{
    m_graphicsDevice = graphicsDevice;

    // �I�[�u�\���p��UI�R���|�[�l���g��������
    m_orbDisplay = std::make_unique<UIOrb>();
    if (!m_orbDisplay->Initialize(m_graphicsDevice))
    {
        return false;
    }

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
    // �eUI�R���|�[�l���g��Update���Ăяo��
    m_orbDisplay->Update(deltaTime);
    // m_minimap��Update�s�v
}

void UI::Render(const Camera* camera, const std::vector<std::unique_ptr<Enemy>>& enemies, const std::vector<std::unique_ptr<Orb>>& orbs)
{
    // �eUI�R���|�[�l���g��Render���Ăяo��
    m_minimap->Render(camera, enemies, orbs);
    m_orbDisplay->Render();
}