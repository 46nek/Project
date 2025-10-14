// minimap.h (この内容で完全に置き換えてください)

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
 * @brief 画面上に2Dのミニマップを描画
 */
class Minimap
{
public:
    Minimap();
    ~Minimap();

    bool Initialize(GraphicsDevice* graphicsDevice, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);
    void Shutdown();
    // Render関数の引数にオーブのリストを追加
    void Render(const Camera* camera, const std::vector<std::unique_ptr<Enemy>>& enemies, const std::vector<std::unique_ptr<Orb>>& orbs);

private:
    GraphicsDevice* m_graphicsDevice;
    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
    std::unique_ptr<Sprite> m_pathSprite;
    std::unique_ptr<Sprite> m_playerSprite;
    std::unique_ptr<Sprite> m_enemySprite;
    std::unique_ptr<Sprite> m_orbSprite; // <--- 追加
    std::unique_ptr<Sprite> m_frameSprite;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_scissorRasterizerState;

    const std::vector<std::vector<MazeGenerator::CellType>>* m_mazeData;
    float m_pathWidth;

    // ミニマップのプロパティ
    DirectX::XMFLOAT2 m_position;
    DirectX::XMFLOAT2 m_viewSize;
    float m_cellSize;
    float m_zoomFactor;
    float m_pathSpriteScale;
    float m_playerSpriteScale;
    float m_enemySpriteScale;
    float m_orbSpriteScale; // <--- 追加
};