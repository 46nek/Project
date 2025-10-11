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

/**
 * @class Minimap
 * @brief ��ʏ��2D�̃~�j�}�b�v��`��
 */
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
    std::unique_ptr<Sprite> m_pathSprite;
    std::unique_ptr<Sprite> m_playerSprite;
    std::unique_ptr<Sprite> m_frameSprite;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_scissorRasterizerState;

    const std::vector<std::vector<MazeGenerator::CellType>>* m_mazeData;

    // �~�j�}�b�v�̃v���p�e�B
    DirectX::XMFLOAT2 m_position;
    DirectX::XMFLOAT2 m_viewSize;
    float m_cellSize;
    float m_zoomFactor;
    float m_pathSpriteScale;
    float m_playerSpriteScale;
};