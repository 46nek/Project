#include "AStar.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <vector>

// 蠎ｧ讓吶ｒ菫晄戟縺吶ｋ縺溘ａ縺ｮ繧ｷ繝ｳ繝励Ν縺ｪ讒矩菴・
struct Coord {
	int x, y;
};

// 蜆ｪ蜈亥ｺｦ莉倥″繧ｭ繝･繝ｼ縺ｧF繧ｳ繧ｹ繝医′譛蟆上・繝弱・繝峨ｒ豈碑ｼ・☆繧九◆繧√・繧ｫ繧ｹ繧ｿ繝繧ｳ繝ｳ繝代Ξ繝ｼ繧ｿ
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

// A*謗｢邏｢繧｢繝ｫ繧ｴ繝ｪ繧ｺ繝縺ｧ譛遏ｭ邨瑚ｷｯ繧定ｦ九▽縺代ｋ (譛驕ｩ蛹悶・螳牙・諤ｧ蜷台ｸ顔沿)
std::vector<DirectX::XMFLOAT2> AStar::FindPath(int startX, int startY, int goalX, int goalY) {
	// --- 菫ｮ豁｣1: 繧ｹ繧ｿ繝ｼ繝亥慍轤ｹ縺ｮ蜴ｳ蟇・メ繧ｧ繝・け ---
	if (!IsWalkable(startX, startY)) {
		// 繧ｹ繧ｿ繝ｼ繝亥慍轤ｹ閾ｪ菴薙′螢∝沂縺ｾ繧翫＠縺ｦ縺・ｋ蝣ｴ蜷医・縺ｩ縺・＠繧医≧繧ゅ↑縺・・縺ｧ遨ｺ繧定ｿ斐☆
		return {};
	}

	// --- 菫ｮ豁｣2: 繧ｴ繝ｼ繝ｫ蝨ｰ轤ｹ縺悟｣√□縺｣縺溷ｴ蜷医・謨第ｸ域蒔鄂ｮ ---
	// 繝励Ξ繧､繝､繝ｼ縺悟｣・圀縺ｫ縺・ｋ縺ｨ縲∬ｨ育ｮ嶺ｸ翫悟｣√・荳ｭ縲榊愛螳壹↓縺ｪ繧翫∫ｧｻ蜍輔＠縺ｦ縺薙↑縺上↑繧九・繧帝亟縺・
	if (!IsWalkable(goalX, goalY)) {
		bool foundNewGoal = false;
		int dx[] = { 0, 0, 1, -1 };
		int dy[] = { 1, -1, 0, 0 };

		// 荳贋ｸ句ｷｦ蜿ｳ繧定ｦ九※縲∵ｭｩ縺代ｋ蝣ｴ謇縺後≠繧後・縺昴％繧剃ｻｮ繧ｴ繝ｼ繝ｫ縺ｫ縺吶ｋ
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
		// 謨第ｸ医＠縺ｦ繧ゅム繝｡縺ｪ繧芽ｫｦ繧√ｋ
		if (!foundNewGoal) { return {}; }
	}

	m_currentSearchId++;
	if (m_currentSearchId == 0) { m_currentSearchId = 1; }

	std::priority_queue<Coord, std::vector<Coord>, CompareNode> openList{ CompareNode(m_nodes) };

	// 繧ｹ繧ｿ繝ｼ繝医ヮ繝ｼ繝峨・險ｭ螳・
	NodeInfo& startNode = m_nodes[startY][startX];

	startNode.ResetValue();
	startNode.searchId = m_currentSearchId;
	startNode.g = 0;
	startNode.h = Heuristic(startX, startY, goalX, goalY);
	startNode.inOpenList = true;

	// --- 菫ｮ豁｣3: 隕ｪ繝弱・繝峨ｒ遒ｺ螳溘↓縲檎┌縺・-1, -1)縲阪↓險ｭ螳・---
	// ResetValue縺ｮ螳溯｣・ｼ上ｌ蟇ｾ遲悶→縺励※縲√％縺薙〒譏守､ｺ逧・↓莉｣蜈･縺励∪縺吶・
	startNode.parent = { -1.0f, -1.0f };

	openList.push({ startX, startY });

	// 謗｢邏｢繝ｫ繝ｼ繝・
	while (!openList.empty()) {
		Coord current = openList.top();
		openList.pop();

		// 繧ｴ繝ｼ繝ｫ縺ｫ蛻ｰ驕斐＠縺溷ｴ蜷・
		if (current.x == goalX && current.y == goalY) {
			std::vector<DirectX::XMFLOAT2> path;
			Coord temp = current;

			// 繧ｹ繧ｿ繝ｼ繝亥慍轤ｹ縺ｾ縺ｧ隕ｪ繧偵◆縺ｩ繧・
			while (temp.x != -1 && temp.y != -1) {
				path.push_back({ (float)temp.x, (float)temp.y });

				// 螳牙・蟇ｾ遲・ 驟榊・螟門盾辣ｧ繝√ぉ繝・け
				if (temp.y < 0 || temp.y >= m_height || temp.x < 0 || temp.x >= m_width) { break; }

				const auto& parentCoord = m_nodes[temp.y][temp.x].parent;

				// 隕ｪ諠・ｱ繧貞叙蠕・
				int px = static_cast<int>(parentCoord.x);
				int py = static_cast<int>(parentCoord.y);

				// 辟｡髯舌Ν繝ｼ繝怜ｯｾ遲・ 隕ｪ縺瑚・蛻・・霄ｫ縺縺｣縺溘ｉ邨ゆｺ・
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

			// ID縺悟商縺・ｴ蜷医・繝ｪ繧ｻ繝・ヨ
			if (neighbor.searchId != m_currentSearchId) {
				neighbor.ResetValue();
				neighbor.searchId = m_currentSearchId;
				// 繝ｪ繧ｻ繝・ヨ逶ｴ蠕後・縺溘ａ縲，losedList縺ｪ縺ｩ縺ｮ繝輔Λ繧ｰ繧Ｇalse縺ｫ縺ｪ縺｣縺ｦ縺・ｋ縺ｯ縺・
			}

			if (neighbor.inClosedList) { continue; }

			int newG = m_nodes[current.y][current.x].g + 1;

			if (!neighbor.inOpenList || newG < neighbor.g) {
				neighbor.g = newG;
				neighbor.h = Heuristic(nextX, nextY, goalX, goalY);
				neighbor.parent = { (float)current.x, (float)current.y }; // 隕ｪ繧偵そ繝・ヨ

				if (!neighbor.inOpenList) {
					neighbor.inOpenList = true;
					openList.push({ nextX, nextY });
				}
			}
		}
	}

	return {};
}
