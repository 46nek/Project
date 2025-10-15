#include "AStar.h"
#include <algorithm>
#include <cmath>

// AStar�R���X�g���N�^: ���H�f�[�^����m�[�h�𐶐�
AStar::AStar(const std::vector<std::vector<MazeGenerator::CellType>>& mazeData)
	: m_mazeData(mazeData)
{
	// ��������K�v�͂���܂���BFindPath���Ńm�[�h�͓��I�Ɉ����܂��B
}

AStar::~AStar()
{
}

bool AStar::IsWalkable(int x, int y) const
{
	if (y < 0 || y >= GetMazeHeight() || x < 0 || x >= GetMazeWidth())
	{
		return false;
	}
	return m_mazeData[y][x] == MazeGenerator::CellType::Path;
}

int AStar::GetMazeWidth() const
{
	if (m_mazeData.empty()) return 0;
	return static_cast<int>(m_mazeData[0].size());
}

int AStar::GetMazeHeight() const
{
	return static_cast<int>(m_mazeData.size());
}

// 2�_�Ԃ̃q���[���X�e�B�b�N�R�X�g�i���苗���j���v�Z
int Heuristic(int x1, int y1, int x2, int y2)
{
	return abs(x1 - x2) + abs(y1 - y2);
}

// A*�T���A���S���Y���ōŒZ�o�H��������
std::vector<DirectX::XMFLOAT2> AStar::FindPath(int startX, int startY, int goalX, int goalY)
{
	// ���H�̕��ƍ������擾
	int width = m_mazeData[0].size();
	int height = m_mazeData.size();

	// �S�Ẵm�[�h��ێ�����}�b�v
	std::vector<Node> allNodes;
	allNodes.reserve(width * height);
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			allNodes.emplace_back(x, y);
		}
	}

	// ���W����m�[�h�ւ̃|�C���^���擾���郉���_��
	auto GetNode = [&](int x, int y) {
		if (x < 0 || x >= width || y < 0 || y >= height) return (Node*)nullptr;
		return &allNodes[y * width + x];
		};

	Node* startNode = GetNode(startX, startY);
	Node* goalNode = GetNode(goalX, goalY);

	std::vector<Node*> openList;
	std::vector<Node*> closedList;

	openList.push_back(startNode);

	while (!openList.empty())
	{
		// openList�̒�����f�R�X�g���ł��Ⴂ�m�[�h��T��
		auto currentNodeIt = std::min_element(openList.begin(), openList.end(), [](Node* a, Node* b) {
			return a->f() < b->f();
			});
		Node* currentNode = *currentNodeIt;

		// �S�[���ɓ��B�����ꍇ
		if (currentNode == goalNode)
		{
			std::vector<DirectX::XMFLOAT2> path;
			while (currentNode != nullptr)
			{
				path.push_back({ (float)currentNode->x, (float)currentNode->y });
				currentNode = currentNode->parent;
			}
			std::reverse(path.begin(), path.end());
			return path;
		}

		// ���݂̃m�[�h��openList����closedList�ֈړ�
		openList.erase(currentNodeIt);
		closedList.push_back(currentNode);

		// �אڃm�[�h�𒲂ׂ� (�㉺���E)
		int dx[] = { 0, 0, 1, -1 };
		int dy[] = { 1, -1, 0, 0 };

		for (int i = 0; i < 4; ++i)
		{
			int nextX = currentNode->x + dx[i];
			int nextY = currentNode->y + dy[i];

			Node* neighbor = GetNode(nextX, nextY);

			// �͈͊O�A�ǁA�܂���closedList�Ɋ܂܂�Ă���ꍇ�̓X�L�b�v
			if (!neighbor || m_mazeData[nextY][nextX] == MazeGenerator::Wall || std::find(closedList.begin(), closedList.end(), neighbor) != closedList.end())
			{
				continue;
			}

			// �V����G�R�X�g���v�Z
			int newG = currentNode->g + 1;

			// openList�ɂȂ��A�܂��͐V�������[�g�̕����R�X�g���Ⴂ�ꍇ
			if (std::find(openList.begin(), openList.end(), neighbor) == openList.end() || newG < neighbor->g)
			{
				neighbor->g = newG;
				neighbor->h = Heuristic(nextX, nextY, goalX, goalY);
				neighbor->parent = currentNode;

				if (std::find(openList.begin(), openList.end(), neighbor) == openList.end())
				{
					openList.push_back(neighbor);
				}
			}
		}
	}

	// �p�X��������Ȃ������ꍇ
	return {};
}