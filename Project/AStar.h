#pragma once
#include <vector>
#include <DirectXMath.h>
#include "MazeGenerator.h"

class AStar
{
public:
	struct Node
	{
		int x, y;
		int g, h;
		Node* parent;

		Node(int x, int y) : x(x), y(y), g(0), h(0), parent(nullptr) {}
		int f() const { return g + h; }
	};

	AStar(const std::vector<std::vector<MazeGenerator::CellType>>& mazeData);
	~AStar();

	std::vector<DirectX::XMFLOAT2> FindPath(int startX, int startY, int goalX, int goalY);

	bool IsWalkable(int x, int y) const;
	int GetMazeWidth() const;
	int GetMazeHeight() const;

private:
	const std::vector<std::vector<MazeGenerator::CellType>>& m_mazeData;
	std::vector<Node*> m_nodes;
};