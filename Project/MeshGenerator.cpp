#include "MeshGenerator.h"

// �w�肳�ꂽ���Ɋ�Â��āA��̎l�p���ʁi2�̎O�p�`�j�𐶐����A���_�ƃC���f�b�N�X�̃��X�g�ɒǉ�����w���p�[�֐�
static void AddFace(
    FaceDirection direction,
    const DirectX::XMFLOAT3& position, // �l�p�`�̒��S�ʒu
    float width, float height,         // �l�p�`�̕��ƍ���
    std::vector<SimpleVertex>& vertices,
    std::vector<unsigned long>& indices)
{
    unsigned long baseIndex = static_cast<unsigned long>(vertices.size());

    SimpleVertex v[4];
    DirectX::XMFLOAT3 normal;
    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;

    // DirectX�̍�����W�n�ł́A�|���S���̒��_�����v���ɒ�`����ƕ\�ʂɂȂ�B
    // �@���́A���̕\�ʂ���O���i�v���C���[���j�������悤�ɒ�`����B
    switch (direction)
    {
    case FaceDirection::Top: // Y+ (��)
        v[0].Pos = { position.x - halfWidth, position.y, position.z + halfHeight };
        v[1].Pos = { position.x + halfWidth, position.y, position.z + halfHeight };
        v[2].Pos = { position.x + halfWidth, position.y, position.z - halfHeight };
        v[3].Pos = { position.x - halfWidth, position.y, position.z - halfHeight };
        normal = { 0.0f, 1.0f, 0.0f };
        break;
    case FaceDirection::Bottom: // Y- (�V��)
        v[0].Pos = { position.x - halfWidth, position.y, position.z - halfHeight };
        v[1].Pos = { position.x + halfWidth, position.y, position.z - halfHeight };
        v[2].Pos = { position.x + halfWidth, position.y, position.z + halfHeight };
        v[3].Pos = { position.x - halfWidth, position.y, position.z + halfHeight };
        normal = { 0.0f, -1.0f, 0.0f };
        break;
    case FaceDirection::Left: // X- (�v���C���[��+X�����猩��̂ŁA�@����+X����������)
        v[0].Pos = { position.x, position.y + halfHeight, position.z - halfWidth };
        v[1].Pos = { position.x, position.y + halfHeight, position.z + halfWidth };
        v[2].Pos = { position.x, position.y - halfHeight, position.z + halfWidth };
        v[3].Pos = { position.x, position.y - halfHeight, position.z - halfWidth };
        normal = { 1.0f, 0.0f, 0.0f };
        break;
    case FaceDirection::Right: // X+ (�v���C���[��-X�����猩��̂ŁA�@����-X����������)
        v[0].Pos = { position.x, position.y + halfHeight, position.z + halfWidth };
        v[1].Pos = { position.x, position.y + halfHeight, position.z - halfWidth };
        v[2].Pos = { position.x, position.y - halfHeight, position.z - halfWidth };
        v[3].Pos = { position.x, position.y - halfHeight, position.z + halfWidth };
        normal = { -1.0f, 0.0f, 0.0f };
        break;
    case FaceDirection::Front: // Z+ (�v���C���[��-Z�����猩��̂ŁA�@����-Z����������)
        v[0].Pos = { position.x - halfWidth, position.y + halfHeight, position.z };
        v[1].Pos = { position.x + halfWidth, position.y + halfHeight, position.z };
        v[2].Pos = { position.x + halfWidth, position.y - halfHeight, position.z };
        v[3].Pos = { position.x - halfWidth, position.y - halfHeight, position.z };
        normal = { 0.0f, 0.0f, -1.0f };
        break;
    case FaceDirection::Back: // Z- (�v���C���[��+Z�����猩��̂ŁA�@����+Z����������)
        v[0].Pos = { position.x + halfWidth, position.y + halfHeight, position.z };
        v[1].Pos = { position.x - halfWidth, position.y + halfHeight, position.z };
        v[2].Pos = { position.x - halfWidth, position.y - halfHeight, position.z };
        v[3].Pos = { position.x + halfWidth, position.y - halfHeight, position.z };
        normal = { 0.0f, 0.0f, 1.0f };
        break;
    }

    v[0].Tex = { 0.0f, 0.0f };
    v[1].Tex = { 1.0f, 0.0f }; // UV���W��0~1�͈̔͂ɐ��K��
    v[2].Tex = { 1.0f, 1.0f }; // UV���W��0~1�͈̔͂ɐ��K��
    v[3].Tex = { 0.0f, 1.0f }; // UV���W��0~1�͈̔͂ɐ��K��

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
        // ���ƓV��̐������W�b�N�͕ύX�Ȃ�
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
        // ������ �ǐ������W�b�N���C�� ������
        for (int y = 0; y < mazeHeight; ++y)
        {
            for (int x = 0; x < mazeWidth; ++x)
            {
                if (mazeData[y][x] == MazeGenerator::Wall) continue;

                DirectX::XMFLOAT3 pos = { x * pathWidth, wallHeight / 2.0f, y * pathWidth };

                // ��̃Z�����ǂȂ�A���݂̃Z���́u���v�̕ǂ�`��
                if (y > 0 && mazeData[y - 1][x] == MazeGenerator::Wall)
                    AddFace(FaceDirection::Back, { pos.x + pathWidth / 2.0f, pos.y, pos.z }, pathWidth, wallHeight, outVertices, outIndices);

                // ���̃Z�����ǂȂ�A���݂̃Z���́u��O�v�̕ǂ�`��
                if (y < mazeHeight - 1 && mazeData[y + 1][x] == MazeGenerator::Wall)
                    AddFace(FaceDirection::Front, { pos.x + pathWidth / 2.0f, pos.y, pos.z + pathWidth }, pathWidth, wallHeight, outVertices, outIndices);

                // ���̃Z�����ǂȂ�A���݂̃Z���́u���v�̕ǂ�`��
                if (x > 0 && mazeData[y][x - 1] == MazeGenerator::Wall)
                    AddFace(FaceDirection::Left, { pos.x, pos.y, pos.z + pathWidth / 2.0f }, pathWidth, wallHeight, outVertices, outIndices);

                // �E�̃Z�����ǂȂ�A���݂̃Z���́u�E�v�̕ǂ�`��
                if (x < mazeWidth - 1 && mazeData[y][x + 1] == MazeGenerator::Wall)
                    AddFace(FaceDirection::Right, { pos.x + pathWidth, pos.y, pos.z + pathWidth / 2.0f }, pathWidth, wallHeight, outVertices, outIndices);
            }
        }
    }

    return true;
}