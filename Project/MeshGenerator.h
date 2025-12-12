#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include "MazeGenerator.h"
#include "Model.h"

// ポリゴンの面の向き
enum class FaceDirection {
	Top, Bottom, Left, Right, Front, Back
};

/**
 * @class MeshGenerator
 * @brief 迷路データから壁、床、天井のメッシュを生成する静的クラス
 */
class MeshGenerator {
public:
	// 生成するメッシュの種類
	enum class MeshType {
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