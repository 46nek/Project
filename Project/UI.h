#pragma once

#include "GraphicsDevice.h"
#include "Minimap.h" 
#include <vector>    

// �O���錾
class Camera;
class Enemy;
class Orb;

/**
 * @class UI
 * @brief �Q�[������UI�v�f�̊Ǘ��ƕ`����s���N���X
 */
class UI
{
public:
    UI();
    ~UI();

    // Initialize�ɖ��H�f�[�^�ƃp�X����ǉ�
    bool Initialize(GraphicsDevice* graphicsDevice, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);
    void Shutdown();
    void Update(float deltaTime); // collectedOrbsCount �������폜
    // Render�ɃJ�����A�G�A�I�[�u�̃��X�g��ǉ�
    void Render(const Camera* camera, const std::vector<std::unique_ptr<Enemy>>& enemies, const std::vector<std::unique_ptr<Orb>>& orbs);

private:
    GraphicsDevice* m_graphicsDevice;
    std::unique_ptr<Minimap> m_minimap;
};