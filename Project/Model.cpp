#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/config.h> 

Model::Model()
{
    m_position = { 0.0f, 0.0f, 0.0f };
    m_rotation = { 0.0f, 0.0f, 0.0f };
    m_scale = { 1.0f, 1.0f, 1.0f };
}

Model::~Model()
{
    Shutdown();
}
bool Model::Initialize(ID3D11Device* device, const std::vector<SimpleVertex>& vertices, const std::vector<unsigned long>& indices)
{
    Mesh newMesh = {};

    if (vertices.empty() || indices.empty())
    {
        return false;
    }

    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
    HRESULT result;

    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(SimpleVertex) * vertices.size());
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexData.pSysMem = vertices.data();
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &newMesh.vertexBuffer);
    if (FAILED(result))
    {
        return false;
    }

    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(unsigned long) * indices.size());
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexData.pSysMem = indices.data();
    result = device->CreateBuffer(&indexBufferDesc, &indexData, &newMesh.indexBuffer);
    if (FAILED(result))
    {
        newMesh.vertexBuffer->Release();
        return false;
    }

    newMesh.indexCount = static_cast<int>(indices.size());
    m_meshes.push_back(newMesh);

    return true;
}

bool Model::Initialize(ID3D11Device* device, const char* modelFilename)
{
    return LoadModel(device, modelFilename);
}

bool Model::LoadTexture(ID3D11Device* device, const wchar_t* textureFilename)
{
    // 既にテクスチャがあれば解放
    if (m_texture)
    {
        m_texture->Shutdown();
        m_texture.reset();
    }

    // 新しいテクスチャを作成して読み込む
    m_texture = std::make_unique<Texture>();
    if (!m_texture->Initialize(device, textureFilename))
    {
        m_texture.reset(); // 失敗したらポインタを空にする
        return false;
    }
    return true;
}


void Model::Shutdown()
{
    // テクスチャを解放
    if (m_texture)
    {
        m_texture->Shutdown();
        m_texture.reset();
    }

    // 全てのメッシュのリソースを解放
    for (auto& mesh : m_meshes)
    {
        if (mesh.vertexBuffer)
        {
            mesh.vertexBuffer->Release();
            mesh.vertexBuffer = nullptr;
        }
        if (mesh.indexBuffer)
        {
            mesh.indexBuffer->Release();
            mesh.indexBuffer = nullptr;
        }
    }
    m_meshes.clear();
}

void Model::Render(ID3D11DeviceContext* deviceContext)
{
    // ↓ モデルのテクスチャをセットする
    if (m_texture)
    {
        ID3D11ShaderResourceView* textureView = m_texture->GetTexture();
        deviceContext->PSSetShaderResources(0, 1, &textureView);
    }

    // すべてのメッシュを描画
    for (auto& mesh : m_meshes)
    {
        RenderBuffers(deviceContext, mesh);
        deviceContext->DrawIndexed(mesh.indexCount, 0, 0);
    }
}

void Model::RenderBuffers(ID3D11DeviceContext* deviceContext, const Mesh& mesh)
{
    unsigned int stride = sizeof(SimpleVertex);
    unsigned int offset = 0;

    // 頂点バッファをセット
    deviceContext->IASetVertexBuffers(0, 1, &mesh.vertexBuffer, &stride, &offset);
    // インデックスバッファをセット
    deviceContext->IASetIndexBuffer(mesh.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    // プリミティブタイプをセット
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool Model::LoadModel(ID3D11Device* device, const std::string& filename)
{
    Assimp::Importer importer;

    importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 180.0f);
    
    const aiScene* scene = importer.ReadFile(filename,
        aiProcess_Triangulate |
        aiProcess_ConvertToLeftHanded |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        const char* errorString = importer.GetErrorString();
        std::wstring wErrorString;
        if (errorString)
        {
            int len = MultiByteToWideChar(CP_ACP, 0, errorString, -1, NULL, 0);
            wErrorString.resize(len);
            MultiByteToWideChar(CP_ACP, 0, errorString, -1, &wErrorString[0], len);
        }
        else
        {
            wErrorString = L"Unknown error.";
        }
        MessageBox(NULL, wErrorString.c_str(), L"Model Load Error", MB_OK);
        return false;
    }

    std::string directory = filename.substr(0, filename.find_last_of('/'));

    ProcessNode(device, scene->mRootNode, scene, directory);

    // FBXにテクスチャが含まれている場合、それをデフォルトとして読み込む試み
    if (scene->HasMaterials() && scene->mMaterials[0]->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        aiString texturePath;
        if (scene->mMaterials[0]->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
        {
            std::string fullPath = directory + "/" + texturePath.C_Str();
            std::wstring wFullPath(fullPath.begin(), fullPath.end());
            LoadTexture(device, wFullPath.c_str());
        }
    }

    return true;
}

void Model::ProcessNode(ID3D11Device* device, aiNode* node, const aiScene* scene, const std::string& directory)
{
    // ノードに含まれるすべてのメッシュを処理
    for (UINT i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_meshes.push_back(ProcessMesh(device, mesh, scene, directory));
    }
    // 子ノードも再帰的に処理
    for (UINT i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(device, node->mChildren[i], scene, directory);
    }
}


Model::Mesh Model::ProcessMesh(ID3D11Device* device, aiMesh* mesh, const aiScene* scene, const std::string& directory)
{
    std::vector<SimpleVertex> vertices;
    std::vector<unsigned long> indices;
    Mesh newMesh = {};

    for (UINT i = 0; i < mesh->mNumVertices; i++)
    {
        SimpleVertex vertex;
        vertex.Pos.x = mesh->mVertices[i].x;
        vertex.Pos.y = mesh->mVertices[i].y;
        vertex.Pos.z = mesh->mVertices[i].z;

        if (mesh->HasTextureCoords(0))
        {
            vertex.Tex.x = mesh->mTextureCoords[0][i].x;
            vertex.Tex.y = mesh->mTextureCoords[0][i].y;
        }
        else
        {
            vertex.Tex = { 0.0f, 0.0f };
        }

        if (mesh->HasNormals())
        {
            vertex.Normal.x = mesh->mNormals[i].x;
            vertex.Normal.y = mesh->mNormals[i].y;
            vertex.Normal.z = mesh->mNormals[i].z;
        }
        else
        {
            vertex.Normal = { 0.0f, 0.0f, 0.0f };
        }

        vertex.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
        vertices.push_back(vertex);
    }

    for (UINT i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (UINT j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    if (vertices.empty() || indices.empty())
    {
        return {}; // データがなければ空のメッシュを返す
    }

    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
    HRESULT result;

    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(SimpleVertex) * vertices.size());
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexData.pSysMem = &vertices[0];
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &newMesh.vertexBuffer);
    if (FAILED(result))
    {
        MessageBox(NULL, L"Failed to create vertex buffer.", L"Model Error", MB_OK);
        return {}; // 失敗したら空のメッシュを返す
    }

    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(unsigned long) * indices.size());
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexData.pSysMem = &indices[0];
    result = device->CreateBuffer(&indexBufferDesc, &indexData, &newMesh.indexBuffer);
    if (FAILED(result))
    {
        newMesh.vertexBuffer->Release(); // 先に作成したバッファを解放
        MessageBox(NULL, L"Failed to create index buffer.", L"Model Error", MB_OK);
        return {}; // 失敗したら空のメッシュを返す
    }

    newMesh.indexCount = static_cast<int>(indices.size());

    return newMesh;
}

void Model::SetPosition(float x, float y, float z) { m_position = { x, y, z }; }
void Model::SetRotation(float x, float y, float z) { m_rotation = { x, y, z }; }
void Model::SetScale(float x, float y, float z) { m_scale = { x, y, z }; }

DirectX::XMMATRIX Model::GetWorldMatrix()
{
    DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
    DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
    return scaleMatrix * rotationMatrix * translationMatrix;
}