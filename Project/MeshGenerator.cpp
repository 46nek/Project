// Project/MeshGenerator.cpp
#include "MeshGenerator.h"

// 指定された情報に基づいて、一つの四角い面（2つの三角形）を生成し、頂点とインデックスのリストに追加するヘルパー関数
static void AddFace(
    FaceDirection direction,
    const DirectX::XMFLOAT3& position, // 四角形の中心位置
    float width, float height,         // 四角形の幅と高さ
    std::vector<SimpleVertex>& vertices,
    std::vector<unsigned long>& indices)
{
    unsigned long baseIndex = static_cast<unsigned long>(vertices.size());

    SimpleVertex v[4];
    DirectX::XMFLOAT3 normal;
    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;

    switch (direction)
    {
    case FaceDirection::Top: // Y+ (上から見て時計回りになるように修正)
        v[0].Pos = { position.x - halfWidth, position.y, position.z + halfHeight };
        v[1].Pos = { position.x + halfWidth, position.y, position.z + halfHeight };
        v[2].Pos = { position.x + halfWidth, position.y, position.z - halfHeight };
        v[3].Pos = { position.x - halfWidth, position.y, position.z - halfHeight };
        normal = { 0.0f, 1.0f, 0.0f };
        break;
    case FaceDirection::Bottom: // Y- (下から見て時計回りになるように修正)
        v[0].Pos = { position.x - halfWidth, position.y, position.z - halfHeight };
        v[1].Pos = { position.x + halfWidth, position.y, position.z - halfHeight };
        v[2].Pos = { position.x + halfWidth, position.y, position.z + halfHeight };
        v[3].Pos = { position.x - halfWidth, position.y, position.z + halfHeight };
        normal = { 0.0f, -1.0f, 0.0f };
        break;
    case FaceDirection::Left: // X-
        v[0].Pos = { position.x, position.y + halfHeight, position.z - halfWidth };
        v[1].Pos = { position.x, position.y + halfHeight, position.z + halfWidth };
        v[2].Pos = { position.x, position.y - halfHeight, position.z + halfWidth };
        v[3].Pos = { position.x, position.y - halfHeight, position.z - halfWidth };
        normal = { -1.0f, 0.0f, 0.0f };
        break;
    case FaceDirection::Right: // X+
        v[0].Pos = { position.x, position.y + halfHeight, position.z + halfWidth };
        v[1].Pos = { position.x, position.y + halfHeight, position.z - halfWidth };
        v[2].Pos = { position.x, position.y - halfHeight, position.z - halfWidth };
        v[3].Pos = { position.x, position.y - halfHeight, position.z + halfWidth };
        normal = { 1.0f, 0.0f, 0.0f };
        break;
    case FaceDirection::Front: // Z+
        v[0].Pos = { position.x - halfWidth, position.y + halfHeight, position.z };
        v[1].Pos = { position.x + halfWidth, position.y + halfHeight, position.z };
        v[2].Pos = { position.x + halfWidth, position.y - halfHeight, position.z };
        v[3].Pos = { position.x - halfWidth, position.y - halfHeight, position.z };
        normal = { 0.0f, 0.0f, 1.0f };
        break;
    case FaceDirection::Back: // Z-
        v[0].Pos = { position.x + halfWidth, position.y + halfHeight, position.z };
        v[1].Pos = { position.x - halfWidth, position.y + halfHeight, position.z };
        v[2].Pos = { position.x - halfWidth, position.y - halfHeight, position.z };
        v[3].Pos = { position.x + halfWidth, position.y - halfHeight, position.z };
        normal = { 0.0f, 0.0f, -1.0f };
        break;
    }

    // UVや法線などの設定は変更なし
    v[0].Tex = { 0.0f, 0.0f };
    v[1].Tex = { width, 0.0f };
    v[2].Tex = { width, height };
    v[3].Tex = { 0.0f, height };

    for (int i = 0; i < 4; ++i)
    {
        v[i].Normal = normal;
        v[i].Color = { 1.0f, 1.0f, 1.0f, 1.0f };
        vertices.push_back(v[i]);
    }

    // インデックスの追加
    indices.push_back(baseIndex);
    indices.push_back(baseIndex + 1);
    indices.push_back(baseIndex + 2);
    indices.push_back(baseIndex);
    indices.push_back(baseIndex + 2);
    indices.push_back(baseIndex + 3);
}

bool MeshGenerator::CreateWallFromMaze(
    const std::vector<std::vector<MazeGenerator::CellType>>& mazeData,
    float pathWidth,
    float wallHeight, // 壁の高さ
    std::vector<SimpleVertex>& outVertices,
    std::vector<unsigned long>& outIndices)
{
    outVertices.clear();
    outIndices.clear();

    if (mazeData.empty()) return false;

    const int mazeHeight = static_cast<int>(mazeData.size());
    const int mazeWidth = static_cast<int>(mazeData[0].size());

    // 処理済みのセルを記録する
    std::vector<std::vector<bool>> visited(mazeHeight, std::vector<bool>(mazeWidth, false));

    // Y軸（縦）方向にスキャン
    for (int y = 0; y < mazeHeight; ++y)
    {
        for (int x = 0; x < mazeWidth; ++x)
        {
            if (mazeData[y][x] == MazeGenerator::Wall || visited[y][x])
            {
                continue;
            }

            // Greedy Meshing: 右方向（X+）にどこまで伸ばせるか調査
            int w = 1;
            while (x + w < mazeWidth && !visited[y][x + w] && mazeData[y][x + w] == MazeGenerator::Path)
            {
                w++;
            }

            // Greedy Meshing: 下方向（Z+）にどこまで伸ばせるか調査
            int h = 1;
            bool canExtend = true;
            while (y + h < mazeHeight && canExtend)
            {
                for (int i = 0; i < w; ++i)
                {
                    if (visited[y + h][x + i] || mazeData[y + h][x + i] == MazeGenerator::Wall)
                    {
                        canExtend = false;
                        break;
                    }
                }
                if (canExtend)
                {
                    h++;
                }
            }

            // 見つけた長方形領域を処理済みとしてマーク
            for (int i = 0; i < h; ++i)
            {
                for (int j = 0; j < w; ++j)
                {
                    visited[y + i][x + j] = true;
                }
            }

            //--- ここで床と天井のメッシュを生成 ---
            float rectWidth = w * pathWidth;
            float rectHeight = h * pathWidth;
            DirectX::XMFLOAT3 centerPos = {
                (x + w / 2.0f) * pathWidth,
                0.0f,
                (y + h / 2.0f) * pathWidth
            };

            // 天井
            centerPos.y = wallHeight;
            AddFace(FaceDirection::Bottom, centerPos, rectWidth, rectHeight, outVertices, outIndices);
        }
    }

    //--- ここで壁のメッシュを生成 ---
    for (int y = 0; y < mazeHeight; ++y)
    {
        for (int x = 0; x < mazeWidth; ++x)
        {
            if (mazeData[y][x] == MazeGenerator::Path)
            {
                DirectX::XMFLOAT3 pos = { x * pathWidth, wallHeight / 2.0f, y * pathWidth };

                // 上のセルをチェック
                if (y > 0 && mazeData[y - 1][x] == MazeGenerator::Wall)
                    AddFace(FaceDirection::Back, { pos.x + pathWidth / 2.0f, pos.y, pos.z }, pathWidth, wallHeight, outVertices, outIndices);
                // 下のセルをチェック
                if (y < mazeHeight - 1 && mazeData[y + 1][x] == MazeGenerator::Wall)
                    AddFace(FaceDirection::Front, { pos.x + pathWidth / 2.0f, pos.y, pos.z + pathWidth }, pathWidth, wallHeight, outVertices, outIndices);
                // 左のセルをチェック
                if (x > 0 && mazeData[y][x - 1] == MazeGenerator::Wall)
                    AddFace(FaceDirection::Left, { pos.x, pos.y, pos.z + pathWidth / 2.0f }, pathWidth, wallHeight, outVertices, outIndices);
                // 右のセルをチェック
                if (x < mazeWidth - 1 && mazeData[y][x + 1] == MazeGenerator::Wall)
                    AddFace(FaceDirection::Right, { pos.x + pathWidth, pos.y, pos.z + pathWidth / 2.0f }, pathWidth, wallHeight, outVertices, outIndices);
            }
        }
    }

    return true;
}