#include "MeshGenerator.h"

static void AddFace(
	FaceDirection direction,
	const DirectX::XMFLOAT3& position,
	float width, float height,
	std::vector<SimpleVertex>& vertices,
	std::vector<unsigned long>& indices) {
	unsigned long baseIndex = static_cast<unsigned long>(vertices.size());

	SimpleVertex v[4];
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 tangent;
	DirectX::XMFLOAT3 binormal;
	float halfWidth = width / 2.0f;
	float halfHeight = height / 2.0f;

	switch (direction) {
	case FaceDirection::Top: // Y+ (蠎・
		v[0].pos = { position.x - halfWidth, position.y, position.z + halfHeight };
		v[1].pos = { position.x + halfWidth, position.y, position.z + halfHeight };
		v[2].pos = { position.x + halfWidth, position.y, position.z - halfHeight };
		v[3].pos = { position.x - halfWidth, position.y, position.z - halfHeight };
		normal = { 0.0f, 1.0f, 0.0f };
		tangent = { 1.0f, 0.0f, 0.0f };
		binormal = { 0.0f, 0.0f, -1.0f };
		break;
	case FaceDirection::Bottom: // Y- (螟ｩ莠・
		v[0].pos = { position.x - halfWidth, position.y, position.z - halfHeight };
		v[1].pos = { position.x + halfWidth, position.y, position.z - halfHeight };
		v[2].pos = { position.x + halfWidth, position.y, position.z + halfHeight };
		v[3].pos = { position.x - halfWidth, position.y, position.z + halfHeight };
		normal = { 0.0f, -1.0f, 0.0f };
		tangent = { 1.0f, 0.0f, 0.0f };
		binormal = { 0.0f, 0.0f, 1.0f };
		break;
	case FaceDirection::Left: // X-
		v[0].pos = { position.x, position.y + halfHeight, position.z - halfWidth };
		v[1].pos = { position.x, position.y + halfHeight, position.z + halfWidth };
		v[2].pos = { position.x, position.y - halfHeight, position.z + halfWidth };
		v[3].pos = { position.x, position.y - halfHeight, position.z - halfWidth };
		normal = { 1.0f, 0.0f, 0.0f };
		tangent = { 0.0f, 0.0f, 1.0f };
		binormal = { 0.0f, -1.0f, 0.0f };
		break;
	case FaceDirection::Right: // X+
		v[0].pos = { position.x, position.y + halfHeight, position.z + halfWidth };
		v[1].pos = { position.x, position.y + halfHeight, position.z - halfWidth };
		v[2].pos = { position.x, position.y - halfHeight, position.z - halfWidth };
		v[3].pos = { position.x, position.y - halfHeight, position.z + halfWidth };
		normal = { -1.0f, 0.0f, 0.0f };
		tangent = { 0.0f, 0.0f, -1.0f, };
		binormal = { 0.0f, -1.0f, 0.0f };
		break;
	case FaceDirection::Front: // Z+
		v[0].pos = { position.x - halfWidth, position.y + halfHeight, position.z };
		v[1].pos = { position.x + halfWidth, position.y + halfHeight, position.z };
		v[2].pos = { position.x + halfWidth, position.y - halfHeight, position.z };
		v[3].pos = { position.x - halfWidth, position.y - halfHeight, position.z };
		normal = { 0.0f, 0.0f, -1.0f };
		tangent = { 1.0f, 0.0f, 0.0f };
		binormal = { 0.0f, -1.0f, 0.0f };
		break;
	case FaceDirection::Back: // Z-
		v[0].pos = { position.x + halfWidth, position.y + halfHeight, position.z };
		v[1].pos = { position.x - halfWidth, position.y + halfHeight, position.z };
		v[2].pos = { position.x - halfWidth, position.y - halfHeight, position.z };
		v[3].pos = { position.x + halfWidth, position.y - halfHeight, position.z };
		normal = { 0.0f, 0.0f, 1.0f };
		tangent = { -1.0f, 0.0f, 0.0f };
		binormal = { 0.0f, -1.0f, 0.0f };
		break;
	}

	v[0].tex = { 0.0f, 0.0f };
	v[1].tex = { 1.0f, 0.0f };
	v[2].tex = { 1.0f, 1.0f };
	v[3].tex = { 0.0f, 1.0f };

	for (int i = 0; i < 4; ++i) {
		v[i].normal = normal;
		v[i].tangent = tangent;
		v[i].binormal = binormal;
		v[i].color = { 1.0f, 1.0f, 1.0f, 1.0f };
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
	std::vector<unsigned long>& outIndices) {
	outVertices.clear();
	outIndices.clear();

	if (mazeData.empty()) { return false; }

	const int mazeHeight = static_cast<int>(mazeData.size());
	const int mazeWidth = static_cast<int>(mazeData[0].size());

	if (type == MeshType::Ceiling || type == MeshType::Floor) {
		// 笆ｼ笆ｼ笆ｼ 縺薙％縺九ｉ縺御ｿｮ豁｣邂・園 笆ｼ笆ｼ笆ｼ
		// Z繝輔ぃ繧､繝・ぅ繝ｳ繧ｰ繧帝亟縺舌◆繧√↓縲∝ｺ翫→螟ｩ莠輔・繝｡繝・す繝･繧偵ｏ縺壹°縺ｫ蟆上＆縺上☆繧・
		const float inset = 0.001f;
		const float faceSize = pathWidth - inset;
		// 笆ｲ笆ｲ笆ｲ 菫ｮ豁｣邂・園縺薙％縺ｾ縺ｧ 笆ｲ笆ｲ笆ｲ

		for (int y = 0; y < mazeHeight; ++y) {
			for (int x = 0; x < mazeWidth; ++x) {
				// 螢√そ繝ｫ縺ｪ繧峨せ繧ｭ繝・・
				if (mazeData[y][x] == MazeGenerator::Wall) { continue; }

				// 騾夊ｷｯ繧ｻ繝ｫ縺ｮ荳ｭ蠢・ｽ咲ｽｮ繧定ｨ育ｮ・
				DirectX::XMFLOAT3 centerPos = { (x + 0.5f) * pathWidth, 0.0f, (y + 0.5f) * pathWidth };

				if (type == MeshType::Ceiling) {
					centerPos.y = wallHeight;
					// 蟆上＆縺上＠縺溘し繧､繧ｺ縺ｧ螟ｩ莠輔・髱｢繧定ｿｽ蜉
					AddFace(FaceDirection::Bottom, centerPos, faceSize, faceSize, outVertices, outIndices);
				}
				else { // type == MeshType::Floor
					// 蟆上＆縺上＠縺溘し繧､繧ｺ縺ｧ蠎翫・髱｢繧定ｿｽ蜉
					AddFace(FaceDirection::Top, centerPos, faceSize, faceSize, outVertices, outIndices);
				}
			}
		}
	}
	else if (type == MeshType::Wall) {
		for (int y = 0; y < mazeHeight; ++y) {
			for (int x = 0; x < mazeWidth; ++x) {
				if (mazeData[y][x] == MazeGenerator::Wall) { continue; }

				DirectX::XMFLOAT3 pos = { x * pathWidth, wallHeight / 2.0f, y * pathWidth };

				if (y > 0 && mazeData[y - 1][x] == MazeGenerator::Wall) {
					AddFace(FaceDirection::Back, { pos.x + pathWidth / 2.0f, pos.y, pos.z }, pathWidth, wallHeight, outVertices, outIndices);
				}

				if (y < mazeHeight - 1 && mazeData[y + 1][x] == MazeGenerator::Wall) {
					AddFace(FaceDirection::Front, { pos.x + pathWidth / 2.0f, pos.y, pos.z + pathWidth }, pathWidth, wallHeight, outVertices, outIndices);
				}

				if (x > 0 && mazeData[y][x - 1] == MazeGenerator::Wall) {
					AddFace(FaceDirection::Left, { pos.x, pos.y, pos.z + pathWidth / 2.0f }, pathWidth, wallHeight, outVertices, outIndices);
				}

				if (x < mazeWidth - 1 && mazeData[y][x + 1] == MazeGenerator::Wall) {
					AddFace(FaceDirection::Right, { pos.x + pathWidth, pos.y, pos.z + pathWidth / 2.0f }, pathWidth, wallHeight, outVertices, outIndices);
				}
			}
		}
	}

	return true;
}
