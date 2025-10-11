#include "AssetLoader.h"
#include "Texture.h" 
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <windows.h>

std::unique_ptr<Model> AssetLoader::LoadModelFromFile(ID3D11Device* device, const std::string& filename)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_GenSmoothNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        MessageBoxA(NULL, importer.GetErrorString(), "Assimp Error", MB_OK);
        return nullptr;
    }

    if (scene->mNumMeshes > 0)
    {
        aiMesh* mesh = scene->mMeshes[0];
        std::vector<SimpleVertex> vertices;
        std::vector<unsigned long> indices;

        for (UINT i = 0; i < mesh->mNumVertices; i++) {
            SimpleVertex vertex{}; // C++11ˆÈ~‚Ì„§‚³‚ê‚é‰Šú‰»
            vertex.Pos = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
            if (mesh->HasNormals()) {
                vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
            }
            if (mesh->HasTextureCoords(0)) {
                vertex.Tex = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
            }
            vertex.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
            vertices.push_back(vertex);
        }

        for (UINT i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (UINT j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        auto model = std::make_unique<Model>();
        if (model->Initialize(device, vertices, indices)) {
            return model;
        }
    }
    return nullptr;
}

std::unique_ptr<Model> AssetLoader::CreateMazeModel(ID3D11Device* device, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth, float wallHeight, MeshGenerator::MeshType type)
{
    std::vector<SimpleVertex> vertices;
    std::vector<unsigned long> indices;
    if (!MeshGenerator::CreateMazeMesh(mazeData, pathWidth, wallHeight, type, vertices, indices)) {
        return nullptr;
    }

    auto model = std::make_unique<Model>();
    if (model->Initialize(device, vertices, indices)) {
        return model;
    }
    return nullptr;
}

std::unique_ptr<Texture> AssetLoader::LoadTexture(ID3D11Device* device, const wchar_t* filename)
{
    auto texture = std::make_unique<Texture>();
    if (!texture->Initialize(device, filename)) {
        return nullptr;
    }
    return texture;
}