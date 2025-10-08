#pragma once
#include <vector>

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

    // –À˜H‚ğ¶¬‚·‚éŠÖ”
    void Generate(int width, int height);

    // ¶¬‚³‚ê‚½–À˜H‚Ìƒf[ƒ^‚ğæ“¾‚·‚éŠÖ”
    const std::vector<std::vector<CellType>>& GetMazeData() const;

private:
    // [‚³—Dæ’Tõ‚Å–À˜H‚ğŒ@‚èi‚ß‚éÄ‹AŠÖ”
    void CarvePath(int x, int y);

    int m_width;  // –À˜H‚Ì•
    int m_height; // –À˜H‚Ì‚‚³
    std::vector<std::vector<CellType>> m_maze; // –À˜H‚Ìƒf[ƒ^‚ğ•Û‚·‚é2ŸŒ³”z—ñ
};