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
    enum class MeshType
    {
        Wall,
        Ceiling,
        Floor
    };

    static bool CreateMazeMesh(
        const std::vector<std::vector<MazeGenerator::CellType>>& mazeData,
        float pathWidth,
        float wallHeight,
        MeshType type,
        std::vector<SimpleVertex>& outVertices,
        std::vector<unsigned long>& outIndices);
};