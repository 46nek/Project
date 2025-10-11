#pragma once
#include <vector>
#include <memory>
#include "GraphicsDevice.h"
#include "MazeGenerator.h"
#include "Sprite.h"
#include "Camera.h"

class Minimap
{
public:
    Minimap();
    ~Minimap();

    bool Initialize(GraphicsDevice* graphicsDevice, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData);
    void Shutdown();
    void Render(const Camera* camera);

private:
    GraphicsDevice* m_graphicsDevice;
    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
    std::unique_ptr<Sprite> m_wallSprite;
    std::unique_ptr<Sprite> m_pathSprite;
    std::unique_ptr<Sprite> m_playerSprite;

    // MazeGenerator����const�Q�ƂƂ��Ď󂯎��
    const std::vector<std::vector<MazeGenerator::CellType>>* m_mazeData;

    // �~�j�}�b�v�̃v���p�e�B
    DirectX::XMFLOAT2 m_position; // ��ʏ�̍�����W
    float m_cellSize;             // 1�}�X�̃s�N�Z���T�C�Y
    float m_spriteScale;          // �X�v���C�g�̊g�k��
    float m_mapWidth;             // �}�b�v�S�̂̕�
    float m_mapHeight;            // �}�b�v�S�̂̍���
};