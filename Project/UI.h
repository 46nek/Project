#pragma once

#include "GraphicsDevice.h"
#include "Minimap.h" 
#include <vector>    

// 前方宣言
class Camera;
class Enemy;
class Orb;

/**
 * @class UI
 * @brief ゲーム内のUI要素の管理と描画を行うクラス
 */
class UI
{
public:
    UI();
    ~UI();

    // Initializeに迷路データとパス幅を追加
    bool Initialize(GraphicsDevice* graphicsDevice, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth);
    void Shutdown();
    void Update(float deltaTime); // collectedOrbsCount 引数を削除
    // Renderにカメラ、敵、オーブのリストを追加
    void Render(const Camera* camera, const std::vector<std::unique_ptr<Enemy>>& enemies, const std::vector<std::unique_ptr<Orb>>& orbs);

private:
    GraphicsDevice* m_graphicsDevice;
    std::unique_ptr<Minimap> m_minimap;
};