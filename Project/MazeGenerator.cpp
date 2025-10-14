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

void MazeGenerator::Generate(int width, int height)
{
    m_width = (width % 2 == 0) ? width + 1 : width;
    m_height = (height % 2 == 0) ? height + 1 : height;

    m_maze.assign(m_height, std::vector<CellType>(m_width, Wall));
    std::vector<std::vector<bool>> protectedCells(m_height, std::vector<bool>(m_width, false));

    // 1. �Œ�̕����̎��ӂ�ی�̈�ɐݒ�
    const int roomSize = 3;
    const int cornerOffset = 1;
    using Rect = std::tuple<int, int, int, int>;
    const std::vector<Rect> rooms = {
        std::make_tuple(cornerOffset, cornerOffset, roomSize, roomSize),
        std::make_tuple(m_width - cornerOffset - roomSize, cornerOffset, roomSize, roomSize),
        std::make_tuple(cornerOffset, m_height - cornerOffset - roomSize, roomSize, roomSize),
        std::make_tuple(m_width - cornerOffset - roomSize, m_height - cornerOffset - roomSize, roomSize, roomSize),
        std::make_tuple((m_width - roomSize) / 2, (m_height - roomSize) / 2, roomSize, roomSize)
    };
    for (const auto& r : rooms) {
        int sx = std::get<0>(r), sy = std::get<1>(r), w = std::get<2>(r), h = std::get<3>(r);
        for (int y = sy - 1; y <= sy + h; ++y) {
            for (int x = sx - 1; x <= sx + w; ++x) {
                if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
                    protectedCells[y][x] = true;
                }
            }
        }
    }

    // 2. �x�[�X�ƂȂ郉���_���Ȗ��H�𐶐�
    GenerateBaseMaze(protectedCells);

    // 3. ���H�𒲐��i�s���~�܂�����炵�A�ʘH���ׂ�����j
    RefineMaze(protectedCells);

    // 4. �Œ�̕�����ʘH���㏑�����Ĕz�u
    AddFeatures();

    // 5. �X�^�[�g�n�_��ݒ�
    m_startX = (m_width - 1) / 2;
    m_startY = (m_height - 1) / 2;
}

void MazeGenerator::GenerateBaseMaze(const std::vector<std::vector<bool>>& protectedCells)
{
    for (int y = 1; y < m_height; y += 2) {
        for (int x = 1; x < m_width; x += 2) {
            CarvePath(x, y, protectedCells);
        }
    }
}

void MazeGenerator::AddFeatures()
{
    const int roomSize = 3;
    const int offset = 1;

    // ����
    CreateRoom(offset, offset, roomSize, roomSize); // ����
    CreateRoom(m_width - offset - roomSize, offset, roomSize, roomSize); // �E��
    CreateRoom(offset, m_height - offset - roomSize, roomSize, roomSize); // ����
    CreateRoom(m_width - offset - roomSize, m_height - offset - roomSize, roomSize, roomSize); // �E��
    CreateRoom((m_width - roomSize) / 2, (m_height - roomSize) / 2, roomSize, roomSize); // ����

    // �O���ʘH
    CreateRoom(offset, offset, m_width - offset * 2, 1); // ��
    CreateRoom(offset, m_height - offset - 1, m_width - offset * 2, 1); // ��
    CreateRoom(offset, offset, 1, m_height - offset * 2); // ��
    CreateRoom(m_width - offset - 1, offset, 1, m_height - offset * 2); // �E

    // ��������̐ڑ��ʘH
    int centerX = m_width / 2;
    int centerY = m_height / 2;
    CreateRoom(centerX, offset, 1, centerY - offset); // ����������
    CreateRoom(centerX, centerY + 1, 1, (m_height - offset - 1) - centerY); // �������牺��
    CreateRoom(offset, centerY, centerX - offset, 1); // �������獶��
    CreateRoom(centerX + 1, centerY, (m_width - offset - 1) - centerX, 1); // ��������E��
}

void MazeGenerator::RefineMaze(const std::vector<std::vector<bool>>& protectedCells)
{
    RemoveDeadEnds(protectedCells);
    ThinPaths(protectedCells);
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

void MazeGenerator::CarvePath(int x, int y, const std::vector<std::vector<bool>>& protectedCells)
{
    if (x <= 0 || y <= 0 || x >= m_width - 1 || y >= m_height - 1 || x % 2 == 0 || y % 2 == 0 || m_maze[y][x] == Path || protectedCells[y][x])
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

            if (nx > 0 && nx < m_width - 1 && ny > 0 && ny < m_height - 1 && m_maze[ny][nx] == Wall && !protectedCells[ny][nx] && !protectedCells[wallY][wallX])
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

void MazeGenerator::RemoveDeadEnds(const std::vector<std::vector<bool>>& protectedCells)
{
    bool needsAnotherPass = true;
    while (needsAnotherPass)
    {
        needsAnotherPass = false;
        std::vector<std::pair<int, int>> deadEnds;

        for (int y = 1; y < m_height - 1; ++y)
        {
            for (int x = 1; x < m_width - 1; ++x)
            {
                if (m_maze[y][x] == Wall) continue;

                int pathNeighbors = 0;
                if (m_maze[y - 1][x] == Path) pathNeighbors++;
                if (m_maze[y + 1][x] == Path) pathNeighbors++;
                if (m_maze[y][x - 1] == Path) pathNeighbors++;
                if (m_maze[y][x + 1] == Path) pathNeighbors++;

                if (pathNeighbors == 1)
                {
                    deadEnds.push_back({ x, y });
                }
            }
        }

        if (!deadEnds.empty())
        {
            needsAnotherPass = true;
            for (const auto& de : deadEnds)
            {
                int x = de.first;
                int y = de.second;

                if (protectedCells[y][x]) continue;

                std::vector<std::pair<int, int>> wallNeighbors;
                if (y > 1 && m_maze[y - 1][x] == Wall && !protectedCells[y - 1][x]) wallNeighbors.push_back({ x, y - 1 });
                if (y < m_height - 2 && m_maze[y + 1][x] == Wall && !protectedCells[y + 1][x]) wallNeighbors.push_back({ x, y + 1 });
                if (x > 1 && m_maze[y][x - 1] == Wall && !protectedCells[y][x - 1]) wallNeighbors.push_back({ x - 1, y });
                if (x < m_width - 2 && m_maze[y][x + 1] == Wall && !protectedCells[y][x + 1]) wallNeighbors.push_back({ x + 1, y });

                if (!wallNeighbors.empty())
                {
                    std::shuffle(wallNeighbors.begin(), wallNeighbors.end(), m_rng);
                    m_maze[wallNeighbors[0].second][wallNeighbors[0].first] = Path;
                }
            }
        }
    }
}

void MazeGenerator::ThinPaths(const std::vector<std::vector<bool>>& protectedCells)
{
    bool changed = true;
    while (changed)
    {
        changed = false;
        for (int y = 0; y < m_height - 1; ++y)
        {
            for (int x = 0; x < m_width - 1; ++x)
            {
                if (m_maze[y][x] == Path &&
                    m_maze[y + 1][x] == Path &&
                    m_maze[y][x + 1] == Path &&
                    m_maze[y + 1][x + 1] == Path)
                {
                    if (!protectedCells[y + 1][x + 1]) { m_maze[y + 1][x + 1] = Wall; changed = true; }
                    else if (!protectedCells[y][x + 1]) { m_maze[y][x + 1] = Wall; changed = true; }
                    else if (!protectedCells[y + 1][x]) { m_maze[y + 1][x] = Wall; changed = true; }
                    else if (!protectedCells[y][x]) { m_maze[y][x] = Wall; changed = true; }
                }
            }
        }
    }
}

const std::vector<std::vector<MazeGenerator::CellType>>& MazeGenerator::GetMazeData() const
{
    return m_maze;
}

std::pair<int, int> MazeGenerator::GetStartPosition() const
{
    return { m_startX, m_startY };
}