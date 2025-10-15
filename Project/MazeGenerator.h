#pragma once
#include <vector>
#include <random>

class MazeGenerator
{
public:
	enum CellType
	{
		Path,
		Wall
	};

	MazeGenerator();
	~MazeGenerator();

	void Generate(int width, int height);
	const std::vector<std::vector<CellType>>& GetMazeData() const;
	std::pair<int, int> GetStartPosition() const;

private:
	// 既存のヘルパー関数
	void CarvePath(int x, int y, const std::vector<std::vector<bool>>& protectedCells);
	void CreateRoom(int x, int y, int width, int height);
	void RemoveDeadEnds(const std::vector<std::vector<bool>>& protectedCells);
	void ThinPaths(const std::vector<std::vector<bool>>& protectedCells);

	// Generate関数から分割された新しいヘルパー関数
	void GenerateBaseMaze(const std::vector<std::vector<bool>>& protectedCells);
	void AddFeatures();
	void RefineMaze(const std::vector<std::vector<bool>>& protectedCells);

	int m_startX;
	int m_startY;
	int m_width;
	int m_height;
	std::vector<std::vector<CellType>> m_maze;
	std::mt19937 m_rng;
};