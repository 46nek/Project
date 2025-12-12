#pragma once
#include <d3d11.h>
#include <string>
#include <vector>
#include <memory>
#include "Model.h"
#include "MazeGenerator.h"
#include "MeshGenerator.h"

/**
 * @class AssetLoader
 * @brief 3Dモデルやテクスチャなどのアセットをファイルから読み込む静的クラス
 */
class AssetLoader {
public:
	/**
	 * @brief Assimpライブラリを使用してモデルファイルを読み込む
	 * @return 読み込みに成功した場合はModelのユニークポインタ
	 */
	static std::unique_ptr<Model> LoadModelFromFile(ID3D11Device* device, const std::string& filename, float uvScale = 1.0f);

	/**
	 * @brief 迷路データから3Dモデルを生成する
	 * @return 生成に成功した場合はModelのユニークポインタ
	 */
	static std::unique_ptr<Model> CreateMazeModel(ID3D11Device* device, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth, float wallHeight, MeshGenerator::MeshType type);

	/**
	 * @brief テクスチャファイルを読み込む
	 * @return 読み込みに成功した場合はTextureのユニークポインタ
	 */
	static std::unique_ptr<Texture> LoadTexture(ID3D11Device* device, const wchar_t* filename);
};