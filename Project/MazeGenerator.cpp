#include "MazeGenerator.h"
#include <vector>

MazeGenerator::MazeGenerator() : m_width(0), m_height(0), m_startX(0), m_startY(0)
{
}

MazeGenerator::~MazeGenerator()
{
}

// 指定された範囲を道にする（部屋や通路を作る）関数
void MazeGenerator::CreateRoom(int startX, int startY, int width, int height)
{
    // マップの範囲外にならないようにチェック
    if (startX < 0 || startY < 0 || startX + width > m_width || startY + height > m_height) return;

    for (int y = startY; y < startY + height; ++y)
    {
        for (int x = startX; x < startX + width; ++x)
        {
            m_maze[y][x] = Path;
        }
    }
}

void MazeGenerator::Generate(int width, int height)
{
    // 幅と高さは21x21に固定
    m_width = 21;
    m_height = 21;

    // 1. 全てのセルを壁で初期化
    m_maze.assign(m_height, std::vector<CellType>(m_width, Wall));

    // 2. 3x3の部屋を配置
    CreateRoom(1, 1, 3, 3);   // 左上
    CreateRoom(17, 1, 3, 3);  // 右上
    CreateRoom(1, 17, 3, 3);  // 左下
    CreateRoom(17, 17, 3, 3); // 右下
    CreateRoom(9, 9, 3, 3);   // 中央

    // 3. 外周の通路を作成
    CreateRoom(4, 2, 13, 1);  // 上
    CreateRoom(4, 18, 13, 1); // 下
    CreateRoom(2, 4, 1, 13);  // 左
    CreateRoom(18, 4, 1, 13); // 右

    // 4. 中央の部屋と外周をつなぐ通路を作成
    CreateRoom(10, 3, 1, 6);  // 上から中央へ
    CreateRoom(10, 12, 1, 6); // 中央から下へ
    CreateRoom(3, 10, 6, 1);  // 左から中央へ
    CreateRoom(12, 10, 6, 1); // 中央から右へ

    // 5. スタート地点を中央の部屋に設定
    m_startX = 10;
    m_startY = 10;
    // ゴールは設定せず、外への出口をなくす
}

const std::vector<std::vector<MazeGenerator::CellType>>& MazeGenerator::GetMazeData() const
{
    return m_maze;
}

std::pair<int, int> MazeGenerator::GetStartPosition() const
{
    return { m_startX, m_startY };
}

// 以下の関数は使用しないので空にします
void MazeGenerator::CarvePath(int x, int y) {}
void MazeGenerator::RemoveDeadEnds() {}
void MazeGenerator::CreateMoreLoops(int count) {}