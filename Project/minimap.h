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

    // MazeGeneratorからconst参照として受け取る
    const std::vector<std::vector<MazeGenerator::CellType>>* m_mazeData;

    // ミニマップのプロパティ
    DirectX::XMFLOAT2 m_position; // 画面上の左上座標
    float m_cellSize;             // 1マスのピクセルサイズ
    float m_spriteScale;          // スプライトの拡縮率
    float m_mapWidth;             // マップ全体の幅
    float m_mapHeight;            // マップ全体の高さ
};