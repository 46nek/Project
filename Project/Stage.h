#pragma once

#include <vector>
#include <memory>
#include "GraphicsDevice.h"
#include "Model.h"
#include "MazeGenerator.h"

/**
 * @class Stage
 * @brief �Q�[���̃X�e�[�W�i���H�A�n�`���f���Ȃǁj�̍\�z�ƊǗ����s���N���X
 */
class Stage
{
public:
    Stage();
    ~Stage();

    bool Initialize(GraphicsDevice* graphicsDevice);
    void Shutdown();

    const std::vector<std::unique_ptr<Model>>& GetModels() const;
    const std::vector<std::vector<MazeGenerator::CellType>>& GetMazeData() const;
    std::pair<int, int> GetStartPosition() const;
    float GetPathWidth() const;

private:
    // �萔
    static constexpr int MAZE_WIDTH = 31;
    static constexpr int MAZE_HEIGHT = 31;
    static constexpr float PATH_WIDTH = 2.5f;
    static constexpr float WALL_HEIGHT = 5.0f;

    std::unique_ptr<MazeGenerator> m_mazeGenerator;
    std::vector<std::unique_ptr<Model>> m_models;
};