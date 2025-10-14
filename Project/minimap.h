// minimap.h (���̓��e�Ŋ��S�ɒu�������Ă�������)

#pragma once
#include <DirectXMath.h>
#include <wrl/client.h>
#include <vector>
#include <memory>
#include "GraphicsDevice.h"
#include "MazeGenerator.h"
#include "Sprite.h"
#include "Camera.h"
#include "SpriteBatch.h"
#include "Enemy.h"
#include "Orb.h" 

/**
 * @class Minimap
 * @brief ��ʏ��2D�̃~�j�}�b�v��`��
 */
class Minimap
{
public:
    Minimap();
    ~Minimap();

    bool Initialize(GraphicsDevice* graphicsDevice, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);
    void Shutdown();
    // Render�֐��̈����ɃI�[�u�̃��X�g��ǉ�
    void Render(const Camera* camera, const std::vector<std::unique_ptr<Enemy>>& enemies, const std::vector<std::unique_ptr<Orb>>& orbs);

private:
    GraphicsDevice* m_graphicsDevice;
    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
    std::unique_ptr<Sprite> m_pathSprite;
    std::unique_ptr<Sprite> m_playerSprite;
    std::unique_ptr<Sprite> m_enemySprite;
    std::unique_ptr<Sprite> m_orbSprite; // <--- �ǉ�
    std::unique_ptr<Sprite> m_frameSprite;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_scissorRasterizerState;

    const std::vector<std::vector<MazeGenerator::CellType>>* m_mazeData;
    float m_pathWidth;

    // �~�j�}�b�v�̃v���p�e�B
    DirectX::XMFLOAT2 m_position;
    DirectX::XMFLOAT2 m_viewSize;
    float m_cellSize;
    float m_zoomFactor;
    float m_pathSpriteScale;
    float m_playerSpriteScale;
    float m_enemySpriteScale;
    float m_orbSpriteScale; // <--- �ǉ�
};