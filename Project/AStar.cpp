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

int Heuristic(int x1, int y1, int x2, int y2) {
	return abs(x1 - x2) + abs(y1 - y2);
}

// A*探索アルゴリズムで最短経路を見つける (最適化・安全性向上版)
std::vector<DirectX::XMFLOAT2> AStar::FindPath(int startX, int startY, int goalX, int goalY) {
	// --- 修正1: スタート地点の厳密チェック ---
	if (!IsWalkable(startX, startY)) {
		// スタート地点自体が壁埋まりしている場合はどうしようもないので空を返す
		return {};
	}

	// --- 修正2: ゴール地点が壁だった場合の救済措置 ---
	// プレイヤーが壁際にいると、計算上「壁の中」判定になり、移動してこなくなるのを防ぐ
	if (!IsWalkable(goalX, goalY)) {
		bool foundNewGoal = false;
		int dx[] = { 0, 0, 1, -1 };
		int dy[] = { 1, -1, 0, 0 };

		// 上下左右を見て、歩ける場所があればそこを仮ゴールにする
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
		// 救済してもダメなら諦める
		if (!foundNewGoal) { return {}; }
	}

	m_currentSearchId++;
	if (m_currentSearchId == 0) { m_currentSearchId = 1; }

	std::priority_queue<Coord, std::vector<Coord>, CompareNode> openList{ CompareNode(m_nodes) };

	// スタートノードの設定
	NodeInfo& startNode = m_nodes[startY][startX];

	startNode.ResetValue();
	startNode.searchId = m_currentSearchId;
	startNode.g = 0;
	startNode.h = Heuristic(startX, startY, goalX, goalY);
	startNode.inOpenList = true;

	// --- 修正3: 親ノードを確実に「無し(-1, -1)」に設定 ---
	// ResetValueの実装漏れ対策として、ここで明示的に代入します。
	startNode.parent = { -1.0f, -1.0f };

	openList.push({ startX, startY });

	// 探索ループ
	while (!openList.empty()) {
		Coord current = openList.top();
		openList.pop();

		// ゴールに到達した場合
		if (current.x == goalX && current.y == goalY) {
			std::vector<DirectX::XMFLOAT2> path;
			Coord temp = current;

			// スタート地点まで親をたどる
			while (temp.x != -1 && temp.y != -1) {
				path.push_back({ (float)temp.x, (float)temp.y });

				// 安全対策: 配列外参照チェック
				if (temp.y < 0 || temp.y >= m_height || temp.x < 0 || temp.x >= m_width) { break; }

				const auto& parentCoord = m_nodes[temp.y][temp.x].parent;

				// 親情報を取得
				int px = static_cast<int>(parentCoord.x);
				int py = static_cast<int>(parentCoord.y);

				// 無限ループ対策: 親が自分自身だったら終了
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

			// IDが古い場合はリセット
			if (neighbor.searchId != m_currentSearchId) {
				neighbor.ResetValue();
				neighbor.searchId = m_currentSearchId;
				// リセット直後のため、ClosedListなどのフラグもfalseになっているはず
			}

			if (neighbor.inClosedList) { continue; }

			int newG = m_nodes[current.y][current.x].g + 1;

			if (!neighbor.inOpenList || newG < neighbor.g) {
				neighbor.g = newG;
				neighbor.h = Heuristic(nextX, nextY, goalX, goalY);
				neighbor.parent = { (float)current.x, (float)current.y }; // 親をセット

				if (!neighbor.inOpenList) {
					neighbor.inOpenList = true;
					openList.push({ nextX, nextY });
				}
			}
		}
	}

	return {};
}