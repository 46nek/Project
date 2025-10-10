#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Model.h"
#include "MazeGenerator.h"
#include "MeshGenerator.h"

class AssetLoader
{
public:
    static std::unique_ptr<Model> LoadModelFromFile(ID3D11Device* device, const std::string& filename);
    static std::unique_ptr<Model> CreateMazeModel(ID3D11Device* device, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth, float wallHeight, MeshGenerator::MeshType type);
    static std::unique_ptr<Texture> LoadTexture(ID3D11Device* device, const wchar_t* filename);
};