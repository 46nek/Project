#pragma once
#include <vector>

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

private:
    void CarvePath(int x, int y);
    void RemoveDeadEnds();
    void CreateMoreLoops(int count);

    int m_width;
    int m_height;
    std::vector<std::vector<CellType>> m_maze;
};