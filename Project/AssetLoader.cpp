#include "AssetLoader.h"
#include "Texture.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <windows.h>

std::unique_ptr<Model> AssetLoader::LoadModelFromFile(ID3D11Device* device, const std::string& filename, float uvScale) {
	Assimp::Importer importer;
	// aiProcess_CalcTangentSpace フラグを追加
	const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		MessageBoxA(nullptr, importer.GetErrorString(), "Assimp Error", MB_OK);
		return nullptr;
	}

	if (scene->mNumMeshes > 0) {
		aiMesh* mesh = scene->mMeshes[0];
		std::vector<SimpleVertex> vertices;
		std::vector<unsigned long> indices;

		for (UINT i = 0; i < mesh->mNumVertices; i++) {
			SimpleVertex vertex = {};
			vertex.pos = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
			if (mesh->HasNormals()) {
				vertex.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
			}
			if (mesh->HasTextureCoords(0)) {
				vertex.tex = { mesh->mTextureCoords[0][i].x * uvScale, mesh->mTextureCoords[0][i].y * uvScale };
			}
			if (mesh->HasTangentsAndBitangents()) { // 接線と従法線のデータを取得
				vertex.tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
				vertex.binormal = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
			}
			vertex.color = { 1.0f, 1.0f, 1.0f, 1.0f };
			vertices.push_back(vertex);
		}

		for (UINT i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (UINT j = 0; j < face.mNumIndices; j++) {
				indices.push_back(face.mIndices[j]);
			}
		}

		auto model = std::make_unique<Model>();
		if (model->Initialize(device, vertices, indices)) {
			return model;
		}
	}
	return nullptr;
}

std::unique_ptr<Model> AssetLoader::CreateMazeModel(ID3D11Device* device, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth, float wallHeight, MeshGenerator::MeshType type) {
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

std::unique_ptr<Texture> AssetLoader::LoadTexture(ID3D11Device* device, const wchar_t* filename) {
	auto texture = std::make_unique<Texture>();
	if (!texture->Initialize(device, filename)) {
		return nullptr;
	}
	return texture;
}