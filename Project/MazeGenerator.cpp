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

    RemoveDeadEnds();

    CreateMoreLoops((m_width * m_height) / 7);
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

void MazeGenerator::RemoveDeadEnds()
{
    std::vector<std::pair<int, int>> deadEnds;

    // 1. 全ての行き止まりのセルを見つける
    //    行き止まり = 道であり、かつ隣接する道が1つしかないセル
    for (int y = 1; y < m_height - 1; y++) {
        for (int x = 1; x < m_width - 1; x++) {
            if (m_maze[y][x] == Path) {
                int pathNeighbors = 0;
                if (m_maze[y + 1][x] == Path) pathNeighbors++;
                if (m_maze[y - 1][x] == Path) pathNeighbors++;
                if (m_maze[y][x + 1] == Path) pathNeighbors++;
                if (m_maze[y][x - 1] == Path) pathNeighbors++;

                if (pathNeighbors == 1) {
                    deadEnds.push_back({ x, y });
                }
            }
        }
    }

    // 2. 見つかった行き止まりの80%をランダムに選んで壁を壊し、ループを作る
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(deadEnds.begin(), deadEnds.end(), gen);

    // この数値を変更することで、迷路の複雑さを調整できます (例: 0.8 = 80%)
    int removals = static_cast<int>(deadEnds.size() * 0.80);

    for (int i = 0; i < removals; ++i) {
        if (deadEnds.empty()) break;

        std::pair<int, int> deadEnd = deadEnds.back();
        deadEnds.pop_back();

        int x = deadEnd.first;
        int y = deadEnd.second;

        // 壊すことができる壁の候補（迷路の外周でない壁）を探す
        std::vector<int> potentialDirections; // 0:上, 1:右, 2:下, 3:左

        if (y > 1 && m_maze[y - 1][x] == Wall) potentialDirections.push_back(0);
        if (x < m_width - 2 && m_maze[y][x + 1] == Wall) potentialDirections.push_back(1);
        if (y < m_height - 2 && m_maze[y + 1][x] == Wall) potentialDirections.push_back(2);
        if (x > 1 && m_maze[y][x - 1] == Wall) potentialDirections.push_back(3);

        if (!potentialDirections.empty()) {
            // 候補の中からランダムに一つ選んで壁を道に変える
            std::uniform_int_distribution<> distrib(0, static_cast<int>(potentialDirections.size() - 1));
            int dirToBreak = potentialDirections[distrib(gen)];

            switch (dirToBreak) {
            case 0: m_maze[y - 1][x] = Path; break; // 上
            case 1: m_maze[y][x + 1] = Path; break; // 右
            case 2: m_maze[y + 1][x] = Path; break; // 下
            case 3: m_maze[y][x - 1] = Path; break; // 左
            }
        }
    }
}

void MazeGenerator::CreateMoreLoops(int count)
{
    std::random_device rd;
    std::mt19937 gen(rd());

    // 指定された回数だけ、壁を壊してループを作る試行を行う
    for (int i = 0; i < count; ++i)
    {
        // 迷路の内部(外周を除く)からランダムな座標を選ぶ
        std::uniform_int_distribution<> distribX(1, m_width - 2);
        std::uniform_int_distribution<> distribY(1, m_height - 2);
        int rx = distribX(gen);
        int ry = distribY(gen);

        // 選んだ座標が壁でなければ、ループを追加する対象にならない
        if (m_maze[ry][rx] != Wall) {
            continue;
        }

        // 壁の左右が道の場合（横方向の道を繋ぐ）
        if (m_maze[ry][rx - 1] == Path && m_maze[ry][rx + 1] == Path) {
            m_maze[ry][rx] = Path;
        }
        // 壁の上下が道の場合（縦方向の道を繋ぐ）
        else if (m_maze[ry - 1][rx] == Path && m_maze[ry + 1][rx] == Path) {
            m_maze[ry][rx] = Path;
        }
    }
}