#include "MeshGenerator.h"

// �w�肳�ꂽ���Ɋ�Â��āA��̎l�p���ʁi2�̎O�p�`�j�𐶐����A���_�ƃC���f�b�N�X�̃��X�g�ɒǉ�����w���p�[�֐�
static void AddFace(
	FaceDirection direction,
	const DirectX::XMFLOAT3& position,
	float width, float height,
	std::vector<SimpleVertex>& vertices,
	std::vector<unsigned long>& indices)
{
	unsigned long baseIndex = static_cast<unsigned long>(vertices.size());

	SimpleVertex v[4];
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 tangent;
	DirectX::XMFLOAT3 binormal;
	float halfWidth = width / 2.0f;
	float halfHeight = height / 2.0f;

	switch (direction)
	{
	case FaceDirection::Top: // Y+ (��)
		v[0].Pos = { position.x - halfWidth, position.y, position.z + halfHeight };
		v[1].Pos = { position.x + halfWidth, position.y, position.z + halfHeight };
		v[2].Pos = { position.x + halfWidth, position.y, position.z - halfHeight };
		v[3].Pos = { position.x - halfWidth, position.y, position.z - halfHeight };
		normal = { 0.0f, 1.0f, 0.0f };
		tangent = { 1.0f, 0.0f, 0.0f };
		binormal = { 0.0f, 0.0f, -1.0f };
		break;
	case FaceDirection::Bottom: // Y- (�V��)
		v[0].Pos = { position.x - halfWidth, position.y, position.z - halfHeight };
		v[1].Pos = { position.x + halfWidth, position.y, position.z - halfHeight };
		v[2].Pos = { position.x + halfWidth, position.y, position.z + halfHeight };
		v[3].Pos = { position.x - halfWidth, position.y, position.z + halfHeight };
		normal = { 0.0f, -1.0f, 0.0f };
		tangent = { 1.0f, 0.0f, 0.0f };
		binormal = { 0.0f, 0.0f, 1.0f };
		break;
	case FaceDirection::Left: // X-
		v[0].Pos = { position.x, position.y + halfHeight, position.z - halfWidth };
		v[1].Pos = { position.x, position.y + halfHeight, position.z + halfWidth };
		v[2].Pos = { position.x, position.y - halfHeight, position.z + halfWidth };
		v[3].Pos = { position.x, position.y - halfHeight, position.z - halfWidth };
		normal = { 1.0f, 0.0f, 0.0f };
		tangent = { 0.0f, 0.0f, 1.0f };
		binormal = { 0.0f, -1.0f, 0.0f };
		break;
	case FaceDirection::Right: // X+
		v[0].Pos = { position.x, position.y + halfHeight, position.z + halfWidth };
		v[1].Pos = { position.x, position.y + halfHeight, position.z - halfWidth };
		v[2].Pos = { position.x, position.y - halfHeight, position.z - halfWidth };
		v[3].Pos = { position.x, position.y - halfHeight, position.z + halfWidth };
		normal = { -1.0f, 0.0f, 0.0f };
		tangent = { 0.0f, 0.0f, -1.0f, };
		binormal = { 0.0f, -1.0f, 0.0f };
		break;
	case FaceDirection::Front: // Z+
		v[0].Pos = { position.x - halfWidth, position.y + halfHeight, position.z };
		v[1].Pos = { position.x + halfWidth, position.y + halfHeight, position.z };
		v[2].Pos = { position.x + halfWidth, position.y - halfHeight, position.z };
		v[3].Pos = { position.x - halfWidth, position.y - halfHeight, position.z };
		normal = { 0.0f, 0.0f, -1.0f };
		tangent = { 1.0f, 0.0f, 0.0f };
		binormal = { 0.0f, -1.0f, 0.0f };
		break;
	case FaceDirection::Back: // Z-
		v[0].Pos = { position.x + halfWidth, position.y + halfHeight, position.z };
		v[1].Pos = { position.x - halfWidth, position.y + halfHeight, position.z };
		v[2].Pos = { position.x - halfWidth, position.y - halfHeight, position.z };
		v[3].Pos = { position.x + halfWidth, position.y - halfHeight, position.z };
		normal = { 0.0f, 0.0f, 1.0f };
		tangent = { -1.0f, 0.0f, 0.0f };
		binormal = { 0.0f, -1.0f, 0.0f };
		break;
	}

	v[0].Tex = { 0.0f, 0.0f };
	v[1].Tex = { 1.0f, 0.0f };
	v[2].Tex = { 1.0f, 1.0f };
	v[3].Tex = { 0.0f, 1.0f };

	for (int i = 0; i < 4; ++i)
	{
		v[i].Normal = normal;
		v[i].Tangent = tangent;
		v[i].Binormal = binormal;
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
		// ������ �������炪�C���ӏ� ������
		// Z�t�@�C�e�B���O��h�����߂ɁA���ƓV��̃��b�V�����킸���ɏ���������
		const float inset = 0.001f;
		const float faceSize = pathWidth - inset;
		// ������ �C���ӏ������܂� ������

		for (int y = 0; y < mazeHeight; ++y)
		{
			for (int x = 0; x < mazeWidth; ++x)
			{
				// �ǃZ���Ȃ�X�L�b�v
				if (mazeData[y][x] == MazeGenerator::Wall) continue;

				// �ʘH�Z���̒��S�ʒu���v�Z
				DirectX::XMFLOAT3 centerPos = { (x + 0.5f) * pathWidth, 0.0f, (y + 0.5f) * pathWidth };

				if (type == MeshType::Ceiling) {
					centerPos.y = wallHeight;
					// �����������T�C�Y�œV��̖ʂ�ǉ�
					AddFace(FaceDirection::Bottom, centerPos, faceSize, faceSize, outVertices, outIndices);
				}
				else { // type == MeshType::Floor
					// �����������T�C�Y�ŏ��̖ʂ�ǉ�
					AddFace(FaceDirection::Top, centerPos, faceSize, faceSize, outVertices, outIndices);
				}
			}
		}
	}
	else if (type == MeshType::Wall)
	{
		for (int y = 0; y < mazeHeight; ++y)
		{
			for (int x = 0; x < mazeWidth; ++x)
			{
				if (mazeData[y][x] == MazeGenerator::Wall) continue;

				DirectX::XMFLOAT3 pos = { x * pathWidth, wallHeight / 2.0f, y * pathWidth };

				if (y > 0 && mazeData[y - 1][x] == MazeGenerator::Wall)
					AddFace(FaceDirection::Back, { pos.x + pathWidth / 2.0f, pos.y, pos.z }, pathWidth, wallHeight, outVertices, outIndices);

				if (y < mazeHeight - 1 && mazeData[y + 1][x] == MazeGenerator::Wall)
					AddFace(FaceDirection::Front, { pos.x + pathWidth / 2.0f, pos.y, pos.z + pathWidth }, pathWidth, wallHeight, outVertices, outIndices);

				if (x > 0 && mazeData[y][x - 1] == MazeGenerator::Wall)
					AddFace(FaceDirection::Left, { pos.x, pos.y, pos.z + pathWidth / 2.0f }, pathWidth, wallHeight, outVertices, outIndices);

				if (x < mazeWidth - 1 && mazeData[y][x + 1] == MazeGenerator::Wall)
					AddFace(FaceDirection::Right, { pos.x + pathWidth, pos.y, pos.z + pathWidth / 2.0f }, pathWidth, wallHeight, outVertices, outIndices);
			}
		}
	}

	return true;
}