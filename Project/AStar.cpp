#include "AStar.h"
#include <algorithm>
#include <cmath>

// AStarコンストラクタ: 迷路データからノードを生成
AStar::AStar(const std::vector<std::vector<MazeGenerator::CellType>>& mazeData)
	: m_mazeData(mazeData)
{
	// 何もする必要はありません。FindPath内でノードは動的に扱います。
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

// 2点間のヒューリスティックコスト（推定距離）を計算
int Heuristic(int x1, int y1, int x2, int y2)
{
	return abs(x1 - x2) + abs(y1 - y2);
}

// A*探索アルゴリズムで最短経路を見つける
std::vector<DirectX::XMFLOAT2> AStar::FindPath(int startX, int startY, int goalX, int goalY)
{
	// 迷路の幅と高さを取得
	int width = m_mazeData[0].size();
	int height = m_mazeData.size();

	// 全てのノードを保持するマップ
	std::vector<Node> allNodes;
	allNodes.reserve(width * height);
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			allNodes.emplace_back(x, y);
		}
	}

	// 座標からノードへのポインタを取得するラムダ式
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
		// openListの中からfコストが最も低いノードを探す
		auto currentNodeIt = std::min_element(openList.begin(), openList.end(), [](Node* a, Node* b) {
			return a->f() < b->f();
			});
		Node* currentNode = *currentNodeIt;

		// ゴールに到達した場合
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

		// 現在のノードをopenListからclosedListへ移動
		openList.erase(currentNodeIt);
		closedList.push_back(currentNode);

		// 隣接ノードを調べる (上下左右)
		int dx[] = { 0, 0, 1, -1 };
		int dy[] = { 1, -1, 0, 0 };

		for (int i = 0; i < 4; ++i)
		{
			int nextX = currentNode->x + dx[i];
			int nextY = currentNode->y + dy[i];

			Node* neighbor = GetNode(nextX, nextY);

			// 範囲外、壁、またはclosedListに含まれている場合はスキップ
			if (!neighbor || m_mazeData[nextY][nextX] == MazeGenerator::Wall || std::find(closedList.begin(), closedList.end(), neighbor) != closedList.end())
			{
				continue;
			}

			// 新しいGコストを計算
			int newG = currentNode->g + 1;

			// openListにない、または新しいルートの方がコストが低い場合
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

	// パスが見つからなかった場合
	return {};
}