#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include "MazeGenerator.h"
#include "Model.h"

// �|���S���̖ʂ̌���
enum class FaceDirection {
    Top, Bottom, Left, Right, Front, Back
};

/**
 * @class MeshGenerator
 * @brief ���H�f�[�^����ǁA���A�V��̃��b�V���𐶐�����ÓI�N���X
 */
class MeshGenerator
{
public:
    // �������郁�b�V���̎��
    enum class MeshType
    {
        Wall, Ceiling, Floor
    };

    static bool CreateMazeMesh(
        const std::vector<std::vector<MazeGenerator::CellType>>& mazeData,
        float pathWidth,
        float wallHeight,
        MeshType type,
        std::vector<SimpleVertex>& outVertices,
        std::vector<unsigned long>& outIndices);
};