#pragma once
#include "MazeGenerator.h"
#include "Model.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

// 繝昴Μ繧ｴ繝ｳ縺ｮ髱｢縺ｮ蜷代″
enum class FaceDirection {
	Top, Bottom, Left, Right, Front, Back
};

/**
 * @class MeshGenerator
 * @brief 霑ｷ霍ｯ繝・・繧ｿ縺九ｉ螢√∝ｺ翫∝､ｩ莠輔・繝｡繝・す繝･繧堤函謌舌☆繧矩撕逧・け繝ｩ繧ｹ
 */
class MeshGenerator {
public:
	// 逕滓・縺吶ｋ繝｡繝・す繝･縺ｮ遞ｮ鬘・
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
