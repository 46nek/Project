#pragma once
#include "Model.h"
#include "MazeGenerator.h"
#include "MeshGenerator.h"
#include <d3d11.h>
#include <string>
#include <vector>
#include <memory>

/**
 * @class AssetLoader
 * @brief 3D繝｢繝・Ν繧・ユ繧ｯ繧ｹ繝√Ε縺ｪ縺ｩ縺ｮ繧｢繧ｻ繝・ヨ繧偵ヵ繧｡繧､繝ｫ縺九ｉ隱ｭ縺ｿ霎ｼ繧髱咏噪繧ｯ繝ｩ繧ｹ
 */
class AssetLoader {
public:
	/**
	 * @brief Assimp繝ｩ繧､繝悶Λ繝ｪ繧剃ｽｿ逕ｨ縺励※繝｢繝・Ν繝輔ぃ繧､繝ｫ繧定ｪｭ縺ｿ霎ｼ繧
	 * @return 隱ｭ縺ｿ霎ｼ縺ｿ縺ｫ謌仙粥縺励◆蝣ｴ蜷医・Model縺ｮ繝ｦ繝九・繧ｯ繝昴う繝ｳ繧ｿ
	 */
	static std::unique_ptr<Model> LoadModelFromFile(ID3D11Device* device, const std::string& filename, float uvScale = 1.0f);

	/**
	 * @brief 霑ｷ霍ｯ繝・・繧ｿ縺九ｉ3D繝｢繝・Ν繧堤函謌舌☆繧・
	 * @return 逕滓・縺ｫ謌仙粥縺励◆蝣ｴ蜷医・Model縺ｮ繝ｦ繝九・繧ｯ繝昴う繝ｳ繧ｿ
	 */
	static std::unique_ptr<Model> CreateMazeModel(ID3D11Device* device, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth, float wallHeight, MeshGenerator::MeshType type);

	/**
	 * @brief 繝・け繧ｹ繝√Ε繝輔ぃ繧､繝ｫ繧定ｪｭ縺ｿ霎ｼ繧
	 * @return 隱ｭ縺ｿ霎ｼ縺ｿ縺ｫ謌仙粥縺励◆蝣ｴ蜷医・Texture縺ｮ繝ｦ繝九・繧ｯ繝昴う繝ｳ繧ｿ
	 */
	static std::unique_ptr<Texture> LoadTexture(ID3D11Device* device, const wchar_t* filename);
};
