#pragma once
#include <DirectXMath.h>
#include <vector>
#include <random>

class MazeGenerator {
public:
	enum CellType {
		Path,
		Wall
	};

	struct Room {
		int x, y, width, height;
		DirectX::XMFLOAT3 center; // 繧ｰ繝ｪ繝・ラ蠎ｧ讓吶・繝ｼ繧ｹ縺ｮ荳ｭ蠢・
	};

	MazeGenerator();
	~MazeGenerator();

	void Generate(int width, int height);
	const std::vector<std::vector<CellType>>& GetMazeData() const;
	std::pair<int, int> GetStartPosition() const;
	const std::vector<Room>& GetRooms() const;

	void SetCell(int x, int y, CellType type);

private:
	// 譌｢蟄倥・繝倥Ν繝代・髢｢謨ｰ
	void CarvePath(int x, int y, const std::vector<std::vector<bool>>& protectedCells);
	void CreateRoom(int x, int y, int width, int height);
	void RemoveDeadEnds(const std::vector<std::vector<bool>>& protectedCells);
	void ThinPaths(const std::vector<std::vector<bool>>& protectedCells);

	// Generate髢｢謨ｰ縺九ｉ蛻・牡縺輔ｌ縺滓眠縺励＞繝倥Ν繝代・髢｢謨ｰ
	void GenerateBaseMaze(const std::vector<std::vector<bool>>& protectedCells);
	void AddFeatures();
	void RefineMaze(const std::vector<std::vector<bool>>& protectedCells);

	int m_startX;
	int m_startY;
	int m_width;
	int m_height;
	std::vector<std::vector<CellType>> m_maze;
	std::vector<Room> m_rooms;
	std::mt19937 m_rng;
};
