#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Model::Model()
{

    m_position = { 0.0f, 0.0f, 0.0f };
    m_rotation = { 0.0f, 0.0f, 0.0f };
    m_scale = { 1.0f, 1.0f, 1.0f };
}

Model::~Model()
{
    // デストラクタでShutdownを呼ぶと、オブジェクトがスコープを抜けた時に自動でリソースが解放されるので安全です。
    Shutdown();
}

bool Model::Initialize(ID3D11Device* device, const char* modelFilename)
{
    return LoadModel(device, modelFilename);
}

void Model::Shutdown()
{
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
        if (mesh.texture)
        {
            mesh.texture->Shutdown();
            delete mesh.texture;
            mesh.texture = nullptr;
        }
    }
    m_meshes.clear(); // ベクターをクリア
}

void Model::Render(ID3D11DeviceContext* deviceContext)
{
    // すべてのメッシュを描画
    for (auto& mesh : m_meshes)
    {
        // メッシュ固有のテクスチャをセット
        if (mesh.texture)
        {
            ID3D11ShaderResourceView* textureView = mesh.texture->GetTexture();
            deviceContext->PSSetShaderResources(0, 1, &textureView);
        }

        // メッシュのバッファをセットして描画
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
    const aiScene* scene = importer.ReadFile(filename,
        aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        // Assimpからのエラーメッセージを取得
        const char* errorString = importer.GetErrorString();
        // エラーメッセージをワイド文字列に変換
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
        // エラーメッセージをMessageBoxで表示
        MessageBox(NULL, wErrorString.c_str(), L"Model Load Error", MB_OK);
        return false;
    }

    // モデルファイルのディレクトリパスを取得
    std::string directory = filename.substr(0, filename.find_last_of('/'));

    ProcessNode(device, scene->mRootNode, scene, directory);
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
    Mesh newMesh = {}; // 新しいメッシュを初期化

    // 頂点データを処理
    for (UINT i = 0; i < mesh->mNumVertices; i++)
    {
        SimpleVertex vertex;
        vertex.Pos.x = mesh->mVertices[i].x;
        vertex.Pos.y = mesh->mVertices[i].y;
        vertex.Pos.z = mesh->mVertices[i].z;

        if (mesh->HasTextureCoords(0)) // テクスチャ座標がある場合
        {
            vertex.Tex.x = mesh->mTextureCoords[0][i].x;
            vertex.Tex.y = mesh->mTextureCoords[0][i].y;
        }
        else
        {
            vertex.Tex = { 0.0f, 0.0f };
        }
        vertex.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
        vertices.push_back(vertex);
    }

    // インデックスデータを処理
    for (UINT i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (UINT j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    // --- ここから頂点バッファとインデックスバッファの作成 ---
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
    // TODO: resultのエラーチェック

    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(unsigned long) * indices.size());
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexData.pSysMem = &indices[0];
    result = device->CreateBuffer(&indexBufferDesc, &indexData, &newMesh.indexBuffer);
    // TODO: resultのエラーチェック

    newMesh.indexCount = static_cast<int>(indices.size());

    // マテリアルとテクスチャの読み込み
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        aiString texturePath;
        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
        {
            newMesh.texture = new Texture();
            std::string fullPath = directory + "/" + texturePath.C_Str();
            // ワイド文字列に変換
            std::wstring wFullPath(fullPath.begin(), fullPath.end());
            if (!newMesh.texture->Initialize(device, wFullPath.c_str()))
            {
                // テクスチャ読み込み失敗時の処理
                delete newMesh.texture;
                newMesh.texture = nullptr;
            }
        }
    }

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