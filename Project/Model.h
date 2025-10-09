#pragma once
#include "Direct3D.h"
#include "Texture.h"
#include <assimp/scene.h>
#include <vector>
#include <string>
#include <memory> 

class Model
{
public:
    Model();
    ~Model();

    bool Initialize(ID3D11Device* device, const char* modelFilename);
    bool LoadTexture(ID3D11Device* device, const wchar_t* textureFilename);
    void Shutdown();
    void Render(ID3D11DeviceContext*);

    void SetPosition(float x, float y, float z);
    void SetRotation(float x, float y, float z);
    void SetScale(float x, float y, float z);
    DirectX::XMMATRIX GetWorldMatrix();

private:
    // メッシュを表す構造体
    struct Mesh {
        ID3D11Buffer* vertexBuffer = nullptr; 
        ID3D11Buffer* indexBuffer = nullptr;  
        int indexCount = 0;
    };

    // Assimpを使ってモデルを読み込む
    bool LoadModel(ID3D11Device* device, const std::string& filename);
    // ProcessNodeとProcessMeshにはテクスチャのパスを解決するためにdirectory引数を追加します
    void ProcessNode(ID3D11Device* device, aiNode* node, const aiScene* scene, const std::string& directory);
    Mesh ProcessMesh(ID3D11Device* device, aiMesh* mesh, const aiScene* scene, const std::string& directory);

    // RenderBuffersは特定のメッシュを描画するように引数を取ります
    void RenderBuffers(ID3D11DeviceContext*, const Mesh& mesh);

private:
    std::vector<Mesh> m_meshes; // モデル内のメッシュを保持するベクター
    std::unique_ptr<Texture> m_texture;
    DirectX::XMFLOAT3 m_position;
    DirectX::XMFLOAT3 m_rotation;
    DirectX::XMFLOAT3 m_scale;
};