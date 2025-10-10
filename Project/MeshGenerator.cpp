#include "MeshGenerator.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <windows.h> // MessageBoxのため

using namespace DirectX;

bool MeshGenerator::CreateWallFromMaze(
    const std::vector<std::vector<MazeGenerator::CellType>>& mazeData,
    float pathWidth,
    std::vector<SimpleVertex>& outVertices,
    std::vector<unsigned long>& outIndices)
{
    outVertices.clear();
    outIndices.clear();

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile("Assets/cube.obj", aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_GenSmoothNormals);
    if (!scene || !scene->mRootNode || !scene->HasMeshes())
    {
        MessageBox(NULL, L"Failed to load cube.obj for maze generation.", L"MeshGenerator Error", MB_OK);
        return false;
    }
    aiMesh* cubeMesh = scene->mMeshes[0];

    unsigned long vertexOffset = 0;

    const int mazeHeight = static_cast<int>(mazeData.size());
    const int mazeWidth = static_cast<int>(mazeData[0].size());

    for (int y = 0; y < mazeHeight; ++y)
    {
        for (int x = 0; x < mazeWidth; ++x)
        {
            if (mazeData[y][x] == MazeGenerator::Wall)
            {
                for (int i = 0; i < 2; ++i)
                {
                    float height = (i == 0) ? 1.0f : 3.0f;
                    XMMATRIX scale = XMMatrixScaling(pathWidth / 2.0f, 1.0f, pathWidth / 2.0f);
                    XMMATRIX translation = XMMatrixTranslation(x * pathWidth, height, y * pathWidth);
                    XMMATRIX transform = scale * translation;

                    // ★ 修正箇所(1/2): 法線変換用の行列を計算
                    // オブジェクトに不均一なスケーリングがかかっている場合、法線は逆転置行列で変換する必要があります。
                    XMMATRIX inverseTranspose = XMMatrixTranspose(XMMatrixInverse(nullptr, transform));

                    for (unsigned int v = 0; v < cubeMesh->mNumVertices; ++v)
                    {
                        SimpleVertex vertex;
                        XMVECTOR pos = XMVectorSet(cubeMesh->mVertices[v].x, cubeMesh->mVertices[v].y, cubeMesh->mVertices[v].z, 1.0f);
                        pos = XMVector3TransformCoord(pos, transform);
                        XMStoreFloat3(&vertex.Pos, pos);

                        if (cubeMesh->HasNormals())
                        {
                            // ★ 修正箇所(2/2): 法線を逆転置行列で正しく変換
                            XMVECTOR normal = XMVectorSet(cubeMesh->mNormals[v].x, cubeMesh->mNormals[v].y, cubeMesh->mNormals[v].z, 0.0f);
                            normal = XMVector3TransformNormal(normal, inverseTranspose);
                            normal = XMVector3Normalize(normal); // 正規化して長さを1に
                            XMStoreFloat3(&vertex.Normal, normal);
                        }

                        if (cubeMesh->HasTextureCoords(0))
                        {
                            vertex.Tex.x = cubeMesh->mTextureCoords[0][v].x;
                            vertex.Tex.y = cubeMesh->mTextureCoords[0][v].y;
                        }
                        vertex.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
                        outVertices.push_back(vertex);
                    }

                    for (unsigned int f = 0; f < cubeMesh->mNumFaces; ++f)
                    {
                        aiFace face = cubeMesh->mFaces[f];
                        for (unsigned int j = 0; j < face.mNumIndices; ++j)
                        {
                            outIndices.push_back(face.mIndices[j] + vertexOffset);
                        }
                    }
                    vertexOffset += cubeMesh->mNumVertices;
                }
            }
        }
    }
    return true;
}