#pragma once
#include "MazeGenerator.h"
#include "Direct3D.h" 
#include <vector>

class MeshGenerator
{
public:
    static bool CreateWallFromMaze(
        const std::vector<std::vector<MazeGenerator::CellType>>& mazeData,
        float pathWidth,
        std::vector<SimpleVertex>& outVertices,
        std::vector<unsigned long>& outIndices);
};