#include "MazeGenerator.h"
#include <random>
#include <algorithm>
#include <vector>

MazeGenerator::MazeGenerator() : m_width(0), m_height(0)
{
}

MazeGenerator::~MazeGenerator()
{
}

void MazeGenerator::Generate(int width, int height)
{
    // 幅と高さは奇数にする（迷路生成アルゴリズムの都合上）
    m_width = (width % 2 == 0) ? width + 1 : width;
    m_height = (height % 2 == 0) ? height + 1 : height;

    // 全てのセルを壁で初期化
    m_maze.assign(m_height, std::vector<CellType>(m_width, Wall));

    // 乱数生成器の準備
    std::random_device rd;
    std::mt19937 gen(rd());

    // スタート地点をランダムに決める（外周のどこか）
    int startX, startY;
    if (gen() % 2 == 0) { // 上下または左右
        startX = (gen() % (m_width / 2)) * 2 + 1;
        startY = (gen() % 2 == 0) ? 0 : m_height - 1;
    }
    else {
        startX = (gen() % 2 == 0) ? 0 : m_width - 1;
        startY = (gen() % (m_height / 2)) * 2 + 1;
    }

    // 壁の外から掘り始めるイメージ
    if (startX == 0) CarvePath(1, startY);
    else if (startX == m_width - 1) CarvePath(m_width - 2, startY);
    else if (startY == 0) CarvePath(startX, 1);
    else CarvePath(startX, m_height - 2);

    // スタート地点を道にする
    m_maze[startY][startX] = Path;

    // ゴール地点をランダムに決める（スタート地点から遠い場所）
    // do-whileループを削除し、ゴールを直接設定します。
    int endX, endY;
    if (gen() % 2 == 0) {
        endX = (gen() % (m_width / 2)) * 2 + 1;
        endY = (startY == 0) ? m_height - 1 : 0;
    }
    else {
        endX = (startX == 0) ? m_width - 1 : 0;
        endY = (gen() % (m_height / 2)) * 2 + 1;
    }
    m_maze[endY][endX] = Path; // ゴール地点を道にする
}

const std::vector<std::vector<MazeGenerator::CellType>>& MazeGenerator::GetMazeData() const
{
    return m_maze;
}

void MazeGenerator::CarvePath(int x, int y)
{
    m_maze[y][x] = Path; // 現在地を道にする

    // 進むことができる方向（上下左右）をリストアップ
    std::vector<int> directions = { 0, 1, 2, 3 }; // 0:上, 1:右, 2:下, 3:左

    // 乱数で方向をシャッフル
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(directions.begin(), directions.end(), gen);

    for (int dir : directions)
    {
        // 2マス先（壁の向こう側）の座標
        int nextX = x, nextY = y;
        int wallX = x, wallY = y; // 間の壁

        switch (dir) {
        case 0: // 上
            nextY -= 2; wallY -= 1;
            break;
        case 1: // 右
            nextX += 2; wallX += 1;
            break;
        case 2: // 下
            nextY += 2; wallY += 1;
            break;
        case 3: // 左
            nextX -= 2; wallX -= 1;
            break;
        }

        // 2マス先が迷路の範囲内で、かつまだ壁（未訪問）であれば
        if (nextX > 0 && nextX < m_width - 1 && nextY > 0 && nextY < m_height - 1 && m_maze[nextY][nextX] == Wall)
        {
            m_maze[wallY][wallX] = Path; // 間の壁を壊す
            CarvePath(nextX, nextY);   // 次のセルへ再帰的に進む
        }
    }
}