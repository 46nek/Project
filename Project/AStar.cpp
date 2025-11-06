
#include "AStar.h"
#include <algorithm>
#include <cmath>
#include <queue> // 優先度付きキューのために追加
#include <vector> // m_nodesのために追加

// 座標を保持するためのシンプルな構造体
struct Coord
{
	int x, y;
};

// 優先度付きキューでFコストが最小のノードを比較するためのカスタムコンパレータ
struct CompareNode
{
	// m_nodes への参照を保持 (NodeInfo が public になったためアクセス可能)
	const std::vector<std::vector<AStar::NodeInfo>>& nodes;
	CompareNode(const std::vector<std::vector<AStar::NodeInfo>>& nodes) : nodes(nodes) {}

	// 優先度付きキューはデフォルトで「大きい」方を優先するため、
	// Fコストが「大きい」方を「小さい」（優先度が低い）として扱う
	bool operator()(const Coord& a, const Coord& b) const
	{
		return nodes[a.y][a.x].f() > nodes[b.y][b.x].f();
	}
};


// AStarコンストラクタ: 迷路データからノード情報を初期化
AStar::AStar(const std::vector<std::vector<MazeGenerator::CellType>>& mazeData)
	: m_mazeData(mazeData)
{
	m_height = static_cast<int>(m_mazeData.size());
	m_width = (m_height > 0) ? static_cast<int>(m_mazeData[0].size()) : 0;

	// メンバー変数のノード情報をリサイズ
	m_nodes.resize(m_height, std::vector<NodeInfo>(m_width));
}

AStar::~AStar()
{
}

bool AStar::IsWalkable(int x, int y) const
{
	if (y < 0 || y >= m_height || x < 0 || x >= m_width)
	{
		return false;
	}
	return m_mazeData[y][x] == MazeGenerator::CellType::Path;
}

int AStar::GetMazeWidth() const
{
	return m_width;
}

int AStar::GetMazeHeight() const
{
	return m_height;
}

// 2点間のヒューリスティックコスト（マンハッタン距離）を計算
int Heuristic(int x1, int y1, int x2, int y2)
{
	return abs(x1 - x2) + abs(y1 - y2);
}

// A*探索アルゴリズムで最短経路を見つける (最適化版)
std::vector<DirectX::XMFLOAT2> AStar::FindPath(int startX, int startY, int goalX, int goalY)
{
	// スタートかゴールが歩けない場所なら空のパスを返す
	if (!IsWalkable(startX, startY) || !IsWalkable(goalX, goalY))
	{
		return {};
	}

	// 1. ノード情報をリセット
	for (int y = 0; y < m_height; ++y)
	{
		for (int x = 0; x < m_width; ++x)
		{
			m_nodes[y][x].Reset();
		}
	}

	// 2. 優先度付きキュー (OpenList) を初期化
	//    (Fコストが最小の Coord を取り出す)
	// ▼▼▼ 初期化構文を () から {} に変更 ▼▼▼
	std::priority_queue<Coord, std::vector<Coord>, CompareNode> openList{ CompareNode(m_nodes) };
	// ▲▲▲ 変更ここまで ▲▲▲

	// 3. スタートノードの設定
	m_nodes[startY][startX].g = 0;
	m_nodes[startY][startX].h = Heuristic(startX, startY, goalX, goalY);
	m_nodes[startY][startX].inOpenList = true;
	openList.push({ startX, startY });

	// 4. 探索ループ
	while (!openList.empty())
	{
		// 5. OpenListからFコストが最小のノードを取得
		Coord current = openList.top();
		openList.pop();

		// 6. 現在ノードをClosedList（探索済み）に追加
		m_nodes[current.y][current.x].inOpenList = false;
		m_nodes[current.y][current.x].inClosedList = true;

		// 7. ゴールに到達した場合、パスを構築して返す
		if (current.x == goalX && current.y == goalY)
		{
			std::vector<DirectX::XMFLOAT2> path;
			Coord temp = current;
			while (temp.x != -1 && temp.y != -1)
			{
				path.push_back({ (float)temp.x, (float)temp.y });
				const auto& parentCoord = m_nodes[temp.y][temp.x].parent;
				temp = { static_cast<int>(parentCoord.x), static_cast<int>(parentCoord.y) };
			}
			std::reverse(path.begin(), path.end());
			return path;
		}

		// 8. 隣接ノードを調べる (上下左右)
		int dx[] = { 0, 0, 1, -1 };
		int dy[] = { 1, -1, 0, 0 };

		for (int i = 0; i < 4; ++i)
		{
			int nextX = current.x + dx[i];
			int nextY = current.y + dy[i];

			// 範囲外、壁、またはClosedListに含まれている場合はスキップ
			if (!IsWalkable(nextX, nextY) || m_nodes[nextY][nextX].inClosedList)
			{
				continue;
			}

			// 9. 新しいGコストを計算
			int newG = m_nodes[current.y][current.x].g + 1; // コストは常に1

			// 10. OpenListにない、または新しいルートの方がコストが低い場合
			if (!m_nodes[nextY][nextX].inOpenList || newG < m_nodes[nextY][nextX].g)
			{
				m_nodes[nextY][nextX].g = newG;
				m_nodes[nextY][nextX].h = Heuristic(nextX, nextY, goalX, goalY);
				m_nodes[nextY][nextX].parent = { (float)current.x, (float)current.y };

				// OpenListになかった場合のみ追加
				if (!m_nodes[nextY][nextX].inOpenList)
				{
					m_nodes[nextY][nextX].inOpenList = true;
					openList.push({ nextX, nextY });
				}
			}
		}
	}

	// 11. パスが見つからなかった場合
	return {};
}