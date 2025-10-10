#pragma once
#include "MazeGenerator.h"
#include "Model.h"
#include <vector>

enum class FaceDirection {
    Top,
    Bottom,
    Left,
    Right,
    Front,
    Back
};

class MeshGenerator
{
public:
    static bool CreateWallFromMaze(
        const std::vector<std::vector<MazeGenerator::CellType>>& mazeData,
        float pathWidth,
        float wallHeight,
        std::vector<SimpleVertex>& outVertices,
        std::vector<unsigned long>& outIndices);
};