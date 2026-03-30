#include "AStar.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <vector>

// 座標を保持するためのシンプルな構造体
struct Coord {
	int x, y;
};

// 優先度付きキューでFコストが最小のノードを比較するためのカスタムコンパレータ
struct CompareNode {
	const std::vector<std::vector<AStar::NodeInfo>>& nodes;
	CompareNode(const std::vector<std::vector<AStar::NodeInfo>>& nodes) : nodes(nodes) {}

	bool operator()(const Coord& a, const Coord& b) const {
		return nodes[a.y][a.x].f() > nodes[b.y][b.x].f();
	}
};

AStar::AStar(const std::vector<std::vector<MazeGenerator::CellType>>& mazeData)
	: m_mazeData(mazeData), m_currentSearchId(0) {
	m_height = static_cast<int>(m_mazeData.size());
	m_width = (m_height > 0) ? static_cast<int>(m_mazeData[0].size()) : 0;

	m_nodes.resize(m_height, std::vector<NodeInfo>(m_width));
}

AStar::~AStar() {
}

bool AStar::IsWalkable(int x, int y) const {
	if (y < 0 || y >= m_height || x < 0 || x >= m_width) {
		return false;
	}
	return m_mazeData[y][x] == MazeGenerator::CellType::Path;
}

int AStar::GetMazeWidth() const {
	return m_width;
}

int AStar::GetMazeHeight() const {
	return m_height;
}

// マンハッタン距離によるヒューリスティック関数
int Heuristic(int x1, int y1, int x2, int y2) {
	return abs(x1 - x2) + abs(y1 - y2);
}

// A*探索アルゴリズムで最短経路を見つける
std::vector<DirectX::XMFLOAT2> AStar::FindPath(int startX, int startY, int goalX, int goalY) {
	// --- スタート地点の厳密チェック ---
	if (!IsWalkable(startX, startY)) {
		return {};
	}

	// --- ゴール地点が壁だった場合の救済措置 ---
	if (!IsWalkable(goalX, goalY)) {
		bool foundNewGoal = false;
		int dx[] = { 0, 0, 1, -1 };
		int dy[] = { 1, -1, 0, 0 };

		for (int i = 0; i < 4; ++i) {
			int nx = goalX + dx[i];
			int ny = goalY + dy[i];
			if (IsWalkable(nx, ny)) {
				goalX = nx;
				goalY = ny;
				foundNewGoal = true;
				break;
			}
		}
		if (!foundNewGoal) { return {}; }
	}

	m_currentSearchId++;
	if (m_currentSearchId == 0) { m_currentSearchId = 1; }

	std::priority_queue<Coord, std::vector<Coord>, CompareNode> openList{ CompareNode(m_nodes) };

	NodeInfo& startNode = m_nodes[startY][startX];
	startNode.ResetValue();
	startNode.searchId = m_currentSearchId;
	startNode.g = 0;
	startNode.h = Heuristic(startX, startY, goalX, goalY);
	startNode.inOpenList = true;

	// --- 親ノードを確実に「ない (-1, -1)」に設定 ---
	startNode.parent = { -1.0f, -1.0f };

	openList.push({ startX, startY });

	while (!openList.empty()) {
		Coord current = openList.top();
		openList.pop();

		if (current.x == goalX && current.y == goalY) {
			std::vector<DirectX::XMFLOAT2> path;
			Coord temp = current;

			while (temp.x != -1 && temp.y != -1) {
				path.push_back({ (float)temp.x, (float)temp.y });

				if (temp.y < 0 || temp.y >= m_height || temp.x < 0 || temp.x >= m_width) { break; }

				const auto& parentCoord = m_nodes[temp.y][temp.x].parent;
				int px = static_cast<int>(parentCoord.x);
				int py = static_cast<int>(parentCoord.y);

				if (px == temp.x && py == temp.y) { break; }
				temp = { px, py };
			}
			std::reverse(path.begin(), path.end());
			return path;
		}

		m_nodes[current.y][current.x].inOpenList = false;
		m_nodes[current.y][current.x].inClosedList = true;

		int dx[] = { 0, 0, 1, -1 };
		int dy[] = { 1, -1, 0, 0 };

		for (int i = 0; i < 4; ++i) {
			int nextX = current.x + dx[i];
			int nextY = current.y + dy[i];

			if (!IsWalkable(nextX, nextY)) { continue; }

			NodeInfo& neighbor = m_nodes[nextY][nextX];

			if (neighbor.searchId != m_currentSearchId) {
				neighbor.ResetValue();
				neighbor.searchId = m_currentSearchId;
			}

			if (neighbor.inClosedList) { continue; }

			int newG = m_nodes[current.y][current.x].g + 1;

			if (!neighbor.inOpenList || newG < neighbor.g) {
				neighbor.g = newG;
				neighbor.h = Heuristic(nextX, nextY, goalX, goalY);
				neighbor.parent = { (float)current.x, (float)current.y };

				if (!neighbor.inOpenList) {
					neighbor.inOpenList = true;
					openList.push({ nextX, nextY });
				}
			}
		}
	}

	return {};
}