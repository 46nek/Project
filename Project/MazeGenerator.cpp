#include "MazeGenerator.h"
#include <vector>
#include <algorithm>
#include <stack>
#include <tuple>

MazeGenerator::MazeGenerator() : m_width(0), m_height(0), m_startX(0), m_startY(0), m_rng(std::random_device{}())
{
}

MazeGenerator::~MazeGenerator()
{
}

void MazeGenerator::CreateRoom(int startX, int startY, int width, int height)
{
    if (startX < 0 || startY < 0 || startX + width > m_width || startY + height > m_height) return;
    for (int y = startY; y < startY + height; ++y) {
        for (int x = startX; x < startX + width; ++x) {
            m_maze[y][x] = Path;
        }
    }
}

void MazeGenerator::CarvePath(int x, int y, const std::vector<std::vector<bool>>& protectedWalls)
{
    if (x <= 0 || y <= 0 || x >= m_width - 1 || y >= m_height - 1 || x % 2 == 0 || y % 2 == 0 || m_maze[y][x] == Path || protectedWalls[y][x])
    {
        return;
    }

    std::stack<std::pair<int, int>> stack;
    stack.push({ x, y });
    m_maze[y][x] = Path;

    int dx[] = { 0, 0, 2, -2 };
    int dy[] = { 2, -2, 0, 0 };
    std::vector<int> dirs = { 0, 1, 2, 3 };

    while (!stack.empty())
    {
        std::pair<int, int> current = stack.top();
        int cx = current.first;
        int cy = current.second;

        std::shuffle(dirs.begin(), dirs.end(), m_rng);

        bool moved = false;
        for (int dir : dirs)
        {
            int nx = cx + dx[dir];
            int ny = cy + dy[dir];
            int wallX = cx + dx[dir] / 2;
            int wallY = cy + dy[dir] / 2;

            if (nx > 0 && nx < m_width - 1 && ny > 0 && ny < m_height - 1 && m_maze[ny][nx] == Wall && !protectedWalls[ny][nx] && !protectedWalls[wallY][wallX])
            {
                m_maze[ny][nx] = Path;
                m_maze[wallY][wallX] = Path;
                stack.push({ nx, ny });
                moved = true;
                break;
            }
        }

        if (!moved)
        {
            stack.pop();
        }
    }
}

void MazeGenerator::CreateMoreLoops(int count, const std::vector<std::vector<bool>>& protectedWalls)
{
    if (count <= 0) return;
    int loopsCreated = 0;
    int attempts = 0;
    int maxAttempts = count * 20;

    while (loopsCreated < count && attempts < maxAttempts)
    {
        attempts++;
        std::uniform_int_distribution<> distribX(1, m_width - 2);
        std::uniform_int_distribution<> distribY(1, m_height - 2);
        int x = distribX(m_rng);
        int y = distribY(m_rng);

        if (protectedWalls[y][x]) continue;

        if (m_maze[y][x] == Wall && (x % 2 != y % 2))
        {
            if (m_maze[y][x - 1] == Path && m_maze[y][x + 1] == Path)
            {
                m_maze[y][x] = Path;
                loopsCreated++;
            }
            else if (m_maze[y - 1][x] == Path && m_maze[y + 1][x] == Path)
            {
                m_maze[y][x] = Path;
                loopsCreated++;
            }
        }
    }
}

void MazeGenerator::Generate(int width, int height)
{
    m_width = (width % 2 == 0) ? width + 1 : width;
    m_height = (height % 2 == 0) ? height + 1 : height;

    m_maze.assign(m_height, std::vector<CellType>(m_width, Wall));
    std::vector<std::vector<bool>> protectedWalls(m_height, std::vector<bool>(m_width, false));

    const int roomSize = 3;
    const int cornerOffset = 1;

    using Rect = std::tuple<int, int, int, int>; // ← これを追加

    // --- 部屋と通路の座標を定義 ---
    const std::vector<Rect> rooms = {
        std::make_tuple(cornerOffset, cornerOffset, roomSize, roomSize),
        std::make_tuple(m_width - cornerOffset - roomSize, cornerOffset, roomSize, roomSize),
        std::make_tuple(cornerOffset, m_height - cornerOffset - roomSize, roomSize, roomSize),
        std::make_tuple(m_width - cornerOffset - roomSize, m_height - cornerOffset - roomSize, roomSize, roomSize),
        std::make_tuple((m_width - roomSize) / 2, (m_height - roomSize) / 2, roomSize, roomSize)
    };

    int centerRoomX = (m_width - roomSize) / 2;
    int centerRoomY = (m_height - roomSize) / 2;
    int outerRoomX = m_width - cornerOffset - roomSize;
    int outerRoomY = m_height - cornerOffset - roomSize;

    const std::vector<Rect> corridors = {
        // Outer corridors
        std::make_tuple(cornerOffset + roomSize, cornerOffset + 1, outerRoomX - (cornerOffset + roomSize), 1),
        std::make_tuple(cornerOffset + roomSize, outerRoomY + 1, outerRoomX - (cornerOffset + roomSize), 1),
        std::make_tuple(cornerOffset + 1, cornerOffset + roomSize, 1, outerRoomY - (cornerOffset + roomSize)),
        std::make_tuple(outerRoomX + 1, cornerOffset + roomSize, 1, outerRoomY - (cornerOffset + roomSize)),
        // Inner corridors
        std::make_tuple((m_width - 1) / 2, cornerOffset + roomSize, 1, centerRoomY - (cornerOffset + roomSize)),
        std::make_tuple((m_width - 1) / 2, centerRoomY + roomSize, 1, outerRoomY - (centerRoomY + roomSize)),
        std::make_tuple(cornerOffset + roomSize, (m_height - 1) / 2, centerRoomX - (cornerOffset + roomSize), 1),
        std::make_tuple(centerRoomX + roomSize, (m_height - 1) / 2, outerRoomX - (centerRoomX + roomSize), 1)
    };

    // --- 骨格の作成 ---
    for (const auto& r : rooms) {
        CreateRoom(std::get<0>(r), std::get<1>(r), std::get<2>(r), std::get<3>(r));
    }
    for (const auto& c : corridors) {
        CreateRoom(std::get<0>(c), std::get<1>(c), std::get<2>(c), std::get<3>(c));
    }

    // --- 外周・中心接続補強 --- ←★追加：通路の端を+1マス掘って貫通させる
    for (const auto& c : corridors) {
        int x = std::get<0>(c);
        int y = std::get<1>(c);
        int w = std::get<2>(c);
        int h = std::get<3>(c);

        // 通路の端を1マス拡張（上下左右）
        if (x == (m_width - 1) / 2) {
            // 縦通路なら上下端を開ける
            if (y > 0) m_maze[y - 1][x] = Path;
            if (y + h < m_height - 1) m_maze[y + h][x] = Path;
        }
        if (y == (m_height - 1) / 2) {
            // 横通路なら左右端を開ける
            if (x > 0) m_maze[y][x - 1] = Path;
            if (x + w < m_width - 1) m_maze[y][x + w] = Path;
        }
    }


    // --- 保護領域の設定 ---
    auto protectStructure = [&](const std::vector<Rect>& structures) {
        for (const auto& s : structures) {
            for (int y = std::get<1>(s) - 1; y <= std::get<1>(s) + std::get<3>(s); ++y) {
                for (int x = std::get<0>(s) - 1; x <= std::get<0>(s) + std::get<2>(s); ++x) {
                    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
                        protectedWalls[y][x] = true;
                    }
                }
            }
        }
        };

    // 部屋と通路を保護
    protectStructure(rooms);
    protectStructure(corridors);

    // --- ランダムな迷路の生成 ---
    for (int y = 1; y < m_height; y += 2) {
        for (int x = 1; x < m_width; x += 2) {
            CarvePath(x, y, protectedWalls);
        }
    }

    // --- ループの追加 ---
    int loopCount = (m_width * m_height) / 20;
    CreateMoreLoops(loopCount, protectedWalls);

    // --- スタート地点の設定 ---
    m_startX = (m_width - 1) / 2;
    m_startY = (m_height - 1) / 2;
}



const std::vector<std::vector<MazeGenerator::CellType>>& MazeGenerator::GetMazeData() const
{
    return m_maze;
}

std::pair<int, int> MazeGenerator::GetStartPosition() const
{
    return { m_startX, m_startY };
}