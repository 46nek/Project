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

    // DirectXの左手座標系では、ポリゴンの頂点を時計回りに定義すると表面になる。
    // 法線は、その表面から外側（プレイヤー側）を向くように定義する。
    switch (direction)
    {
    case FaceDirection::Top: // Y+ (床)
        v[0].Pos = { position.x - halfWidth, position.y, position.z + halfHeight };
        v[1].Pos = { position.x + halfWidth, position.y, position.z + halfHeight };
        v[2].Pos = { position.x + halfWidth, position.y, position.z - halfHeight };
        v[3].Pos = { position.x - halfWidth, position.y, position.z - halfHeight };
        normal = { 0.0f, 1.0f, 0.0f };
        break;
    case FaceDirection::Bottom: // Y- (天井)
        v[0].Pos = { position.x - halfWidth, position.y, position.z - halfHeight };
        v[1].Pos = { position.x + halfWidth, position.y, position.z - halfHeight };
        v[2].Pos = { position.x + halfWidth, position.y, position.z + halfHeight };
        v[3].Pos = { position.x - halfWidth, position.y, position.z + halfHeight };
        normal = { 0.0f, -1.0f, 0.0f };
        break;
    case FaceDirection::Left: // X- (プレイヤーは+X側から見るので、法線は+X方向を向く)
        v[0].Pos = { position.x, position.y + halfHeight, position.z - halfWidth };
        v[1].Pos = { position.x, position.y + halfHeight, position.z + halfWidth };
        v[2].Pos = { position.x, position.y - halfHeight, position.z + halfWidth };
        v[3].Pos = { position.x, position.y - halfHeight, position.z - halfWidth };
        normal = { 1.0f, 0.0f, 0.0f };
        break;
    case FaceDirection::Right: // X+ (プレイヤーは-X側から見るので、法線は-X方向を向く)
        v[0].Pos = { position.x, position.y + halfHeight, position.z + halfWidth };
        v[1].Pos = { position.x, position.y + halfHeight, position.z - halfWidth };
        v[2].Pos = { position.x, position.y - halfHeight, position.z - halfWidth };
        v[3].Pos = { position.x, position.y - halfHeight, position.z + halfWidth };
        normal = { -1.0f, 0.0f, 0.0f };
        break;
    case FaceDirection::Front: // Z+ (プレイヤーは-Z側から見るので、法線は-Z方向を向く)
        v[0].Pos = { position.x - halfWidth, position.y + halfHeight, position.z };
        v[1].Pos = { position.x + halfWidth, position.y + halfHeight, position.z };
        v[2].Pos = { position.x + halfWidth, position.y - halfHeight, position.z };
        v[3].Pos = { position.x - halfWidth, position.y - halfHeight, position.z };
        normal = { 0.0f, 0.0f, -1.0f };
        break;
    case FaceDirection::Back: // Z- (プレイヤーは+Z側から見るので、法線は+Z方向を向く)
        v[0].Pos = { position.x + halfWidth, position.y + halfHeight, position.z };
        v[1].Pos = { position.x - halfWidth, position.y + halfHeight, position.z };
        v[2].Pos = { position.x - halfWidth, position.y - halfHeight, position.z };
        v[3].Pos = { position.x + halfWidth, position.y - halfHeight, position.z };
        normal = { 0.0f, 0.0f, 1.0f };
        break;
    }

    v[0].Tex = { 0.0f, 0.0f };
    v[1].Tex = { 1.0f, 0.0f }; // UV座標を0~1の範囲に正規化
    v[2].Tex = { 1.0f, 1.0f }; // UV座標を0~1の範囲に正規化
    v[3].Tex = { 0.0f, 1.0f }; // UV座標を0~1の範囲に正規化

    for (int i = 0; i < 4; ++i)
    {
        v[i].Normal = normal;
        v[i].Color = { 1.0f, 1.0f, 1.0f, 1.0f };
        vertices.push_back(v[i]);
    }

    indices.push_back(baseIndex);
    indices.push_back(baseIndex + 1);
    indices.push_back(baseIndex + 2);
    indices.push_back(baseIndex);
    indices.push_back(baseIndex + 2);
    indices.push_back(baseIndex + 3);
}

bool MeshGenerator::CreateMazeMesh(
    const std::vector<std::vector<MazeGenerator::CellType>>& mazeData,
    float pathWidth,
    float wallHeight,
    MeshType type,
    std::vector<SimpleVertex>& outVertices,
    std::vector<unsigned long>& outIndices)
{
    outVertices.clear();
    outIndices.clear();

    if (mazeData.empty()) return false;

    const int mazeHeight = static_cast<int>(mazeData.size());
    const int mazeWidth = static_cast<int>(mazeData[0].size());

    if (type == MeshType::Ceiling || type == MeshType::Floor)
    {
        // 床と天井の生成ロジックは変更なし
        std::vector<std::vector<bool>> visited(mazeHeight, std::vector<bool>(mazeWidth, false));
        for (int y = 0; y < mazeHeight; ++y) {
            for (int x = 0; x < mazeWidth; ++x) {
                if (mazeData[y][x] == MazeGenerator::Wall || visited[y][x]) continue;
                int w = 1;
                while (x + w < mazeWidth && !visited[y][x + w] && mazeData[y][x + w] == MazeGenerator::Path) w++;
                int h = 1;
                bool canExtend = true;
                while (y + h < mazeHeight && canExtend) {
                    for (int i = 0; i < w; ++i) {
                        if (visited[y + h][x + i] || mazeData[y + h][x + i] == MazeGenerator::Wall) {
                            canExtend = false;
                            break;
                        }
                    }
                    if (canExtend) h++;
                }
                for (int i = 0; i < h; ++i) for (int j = 0; j < w; ++j) visited[y + i][x + j] = true;
                float rectWidth = w * pathWidth;
                float rectHeight = h * pathWidth;
                DirectX::XMFLOAT3 centerPos = { (x + w / 2.0f) * pathWidth, 0.0f, (y + h / 2.0f) * pathWidth };
                if (type == MeshType::Ceiling) {
                    centerPos.y = wallHeight;
                    AddFace(FaceDirection::Bottom, centerPos, rectWidth, rectHeight, outVertices, outIndices);
                }
                else {
                    centerPos.y = 0.0f;
                    AddFace(FaceDirection::Top, centerPos, rectWidth, rectHeight, outVertices, outIndices);
                }
            }
        }
    }
    else if (type == MeshType::Wall)
    {
        // ▼▼▼ 壁生成ロジックを修正 ▼▼▼
        for (int y = 0; y < mazeHeight; ++y)
        {
            for (int x = 0; x < mazeWidth; ++x)
            {
                if (mazeData[y][x] == MazeGenerator::Wall) continue;

                DirectX::XMFLOAT3 pos = { x * pathWidth, wallHeight / 2.0f, y * pathWidth };

                // 上のセルが壁なら、現在のセルの「奥」の壁を描画
                if (y > 0 && mazeData[y - 1][x] == MazeGenerator::Wall)
                    AddFace(FaceDirection::Back, { pos.x + pathWidth / 2.0f, pos.y, pos.z }, pathWidth, wallHeight, outVertices, outIndices);

                // 下のセルが壁なら、現在のセルの「手前」の壁を描画
                if (y < mazeHeight - 1 && mazeData[y + 1][x] == MazeGenerator::Wall)
                    AddFace(FaceDirection::Front, { pos.x + pathWidth / 2.0f, pos.y, pos.z + pathWidth }, pathWidth, wallHeight, outVertices, outIndices);

                // 左のセルが壁なら、現在のセルの「左」の壁を描画
                if (x > 0 && mazeData[y][x - 1] == MazeGenerator::Wall)
                    AddFace(FaceDirection::Left, { pos.x, pos.y, pos.z + pathWidth / 2.0f }, pathWidth, wallHeight, outVertices, outIndices);

                // 右のセルが壁なら、現在のセルの「右」の壁を描画
                if (x < mazeWidth - 1 && mazeData[y][x + 1] == MazeGenerator::Wall)
                    AddFace(FaceDirection::Right, { pos.x + pathWidth, pos.y, pos.z + pathWidth / 2.0f }, pathWidth, wallHeight, outVertices, outIndices);
            }
        }
    }

    return true;
}