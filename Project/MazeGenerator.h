#pragma once
#include <vector>
#include <random>

/**
 * @class MazeGenerator
 * @brief [‚³—Dæ’TõƒAƒ‹ƒSƒŠƒYƒ€‚ğ—p‚¢‚Ä–À˜H‚ğ©“®¶¬
 */
class MazeGenerator
{
public:
    // –À˜H‚ÌƒZƒ‹‚Ìí—Ş
    enum CellType
    {
        Path, // “¹
        Wall  // •Ç
    };

    MazeGenerator();
    ~MazeGenerator();

    void Generate(int width, int height);
    const std::vector<std::vector<CellType>>& GetMazeData() const;
    std::pair<int, int> GetStartPosition() const;

private:
    void CarvePath(int x, int y, const std::vector<std::vector<bool>>& protectedWalls);
    void CreateMoreLoops(int count, const std::vector<std::vector<bool>>& protectedWalls);
    void CreateRoom(int x, int y, int width, int height);

    int m_startX;
    int m_startY;
    int m_width;
    int m_height;
    std::vector<std::vector<CellType>> m_maze;
    std::mt19937 m_rng;
};