// Project/AStar.h (この内容で完全に置き換えてください)

#pragma once

#include <vector>
#include <DirectXMath.h>
#include "MazeGenerator.h"

// ノード構造体は AStar.cpp へ移動

class AStar
{
public:
	AStar(const std::vector<std::vector<MazeGenerator::CellType>>& mazeData);
	~AStar();

	std::vector<DirectX::XMFLOAT2> FindPath(int startX, int startY, int goalX, int goalY);
	bool IsWalkable(int x, int y) const;
	int GetMazeWidth() const;
	int GetMazeHeight() const;

	// ノード情報を保持する構造体 (AStar.cpp から参照するため public にする)
	struct NodeInfo
	{
		int g = 0; // スタートからの移動コスト
		int h = 0; // ゴールまでの推定コスト
		DirectX::XMFLOAT2 parent = { -1, -1 };
		bool inOpenList = false;
		bool inClosedList = false;

		int f() const { return g + h; }
		void Reset()
		{
			g = 0;
			h = 0;
			parent = { -1, -1 };
			inOpenList = false;
			inClosedList = false;
		}
	};

private:
	const std::vector<std::vector<MazeGenerator::CellType>>& m_mazeData;
	int m_width;
	int m_height;

	// ノード情報をクラスメンバーとして保持 (FindPath呼び出しごとに確保するのを避ける)
	std::vector<std::vector<NodeInfo>> m_nodes;
};