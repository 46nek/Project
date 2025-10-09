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
    // ���ƍ����͊�ɂ���i���H�����A���S���Y���̓s����j
    m_width = (width % 2 == 0) ? width + 1 : width;
    m_height = (height % 2 == 0) ? height + 1 : height;

    // �S�ẴZ����ǂŏ�����
    m_maze.assign(m_height, std::vector<CellType>(m_width, Wall));

    // ����������̏���
    std::random_device rd;
    std::mt19937 gen(rd());

    // �X�^�[�g�n�_�������_���Ɍ��߂�i�O���̂ǂ����j
    int startX, startY;
    if (gen() % 2 == 0) { // �㉺�܂��͍��E
        startX = (gen() % (m_width / 2)) * 2 + 1;
        startY = (gen() % 2 == 0) ? 0 : m_height - 1;
    }
    else {
        startX = (gen() % 2 == 0) ? 0 : m_width - 1;
        startY = (gen() % (m_height / 2)) * 2 + 1;
    }

    // �ǂ̊O����@��n�߂�C���[�W
    if (startX == 0) CarvePath(1, startY);
    else if (startX == m_width - 1) CarvePath(m_width - 2, startY);
    else if (startY == 0) CarvePath(startX, 1);
    else CarvePath(startX, m_height - 2);

    // �X�^�[�g�n�_�𓹂ɂ���
    m_maze[startY][startX] = Path;

    // �S�[���n�_�������_���Ɍ��߂�i�X�^�[�g�n�_���牓���ꏊ�j
    int endX, endY;
    if (gen() % 2 == 0) {
        endX = (gen() % (m_width / 2)) * 2 + 1;
        endY = (startY == 0) ? m_height - 1 : 0;
    }
    else {
        endX = (startX == 0) ? m_width - 1 : 0;
        endY = (gen() % (m_height / 2)) * 2 + 1;
    }
    m_maze[endY][endX] = Path; // �S�[���n�_�𓹂ɂ���

    RemoveDeadEnds();

    CreateMoreLoops((m_width * m_height) / 7);
}

const std::vector<std::vector<MazeGenerator::CellType>>& MazeGenerator::GetMazeData() const
{
    return m_maze;
}

void MazeGenerator::CarvePath(int x, int y)
{
    m_maze[y][x] = Path; // ���ݒn�𓹂ɂ���

    // �i�ނ��Ƃ��ł�������i�㉺���E�j�����X�g�A�b�v
    std::vector<int> directions = { 0, 1, 2, 3 }; // 0:��, 1:�E, 2:��, 3:��

    // �����ŕ������V���b�t��
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(directions.begin(), directions.end(), gen);

    for (int dir : directions)
    {
        // 2�}�X��i�ǂ̌��������j�̍��W
        int nextX = x, nextY = y;
        int wallX = x, wallY = y; // �Ԃ̕�

        switch (dir) {
        case 0: // ��
            nextY -= 2; wallY -= 1;
            break;
        case 1: // �E
            nextX += 2; wallX += 1;
            break;
        case 2: // ��
            nextY += 2; wallY += 1;
            break;
        case 3: // ��
            nextX -= 2; wallX -= 1;
            break;
        }

        // 2�}�X�悪���H�͈͓̔��ŁA���܂��ǁi���K��j�ł����
        if (nextX > 0 && nextX < m_width - 1 && nextY > 0 && nextY < m_height - 1 && m_maze[nextY][nextX] == Wall)
        {
            m_maze[wallY][wallX] = Path; // �Ԃ̕ǂ���
            CarvePath(nextX, nextY);   // ���̃Z���֍ċA�I�ɐi��
        }
    }
}

void MazeGenerator::RemoveDeadEnds()
{
    std::vector<std::pair<int, int>> deadEnds;

    // 1. �S�Ă̍s���~�܂�̃Z����������
    //    �s���~�܂� = ���ł���A���אڂ��铹��1�����Ȃ��Z��
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

    // 2. ���������s���~�܂��80%�������_���ɑI��ŕǂ��󂵁A���[�v�����
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(deadEnds.begin(), deadEnds.end(), gen);

    // ���̐��l��ύX���邱�ƂŁA���H�̕��G���𒲐��ł��܂� (��: 0.8 = 80%)
    int removals = static_cast<int>(deadEnds.size() * 0.80);

    for (int i = 0; i < removals; ++i) {
        if (deadEnds.empty()) break;

        std::pair<int, int> deadEnd = deadEnds.back();
        deadEnds.pop_back();

        int x = deadEnd.first;
        int y = deadEnd.second;

        // �󂷂��Ƃ��ł���ǂ̌��i���H�̊O���łȂ��ǁj��T��
        std::vector<int> potentialDirections; // 0:��, 1:�E, 2:��, 3:��

        if (y > 1 && m_maze[y - 1][x] == Wall) potentialDirections.push_back(0);
        if (x < m_width - 2 && m_maze[y][x + 1] == Wall) potentialDirections.push_back(1);
        if (y < m_height - 2 && m_maze[y + 1][x] == Wall) potentialDirections.push_back(2);
        if (x > 1 && m_maze[y][x - 1] == Wall) potentialDirections.push_back(3);

        if (!potentialDirections.empty()) {
            // ���̒����烉���_���Ɉ�I��ŕǂ𓹂ɕς���
            std::uniform_int_distribution<> distrib(0, static_cast<int>(potentialDirections.size() - 1));
            int dirToBreak = potentialDirections[distrib(gen)];

            switch (dirToBreak) {
            case 0: m_maze[y - 1][x] = Path; break; // ��
            case 1: m_maze[y][x + 1] = Path; break; // �E
            case 2: m_maze[y + 1][x] = Path; break; // ��
            case 3: m_maze[y][x - 1] = Path; break; // ��
            }
        }
    }
}

void MazeGenerator::CreateMoreLoops(int count)
{
    std::random_device rd;
    std::mt19937 gen(rd());

    // �w�肳�ꂽ�񐔂����A�ǂ��󂵂ă��[�v����鎎�s���s��
    for (int i = 0; i < count; ++i)
    {
        // ���H�̓���(�O��������)���烉���_���ȍ��W��I��
        std::uniform_int_distribution<> distribX(1, m_width - 2);
        std::uniform_int_distribution<> distribY(1, m_height - 2);
        int rx = distribX(gen);
        int ry = distribY(gen);

        // �I�񂾍��W���ǂłȂ���΁A���[�v��ǉ�����ΏۂɂȂ�Ȃ�
        if (m_maze[ry][rx] != Wall) {
            continue;
        }

        // �ǂ̍��E�����̏ꍇ�i�������̓����q���j
        if (m_maze[ry][rx - 1] == Path && m_maze[ry][rx + 1] == Path) {
            m_maze[ry][rx] = Path;
        }
        // �ǂ̏㉺�����̏ꍇ�i�c�����̓����q���j
        else if (m_maze[ry - 1][rx] == Path && m_maze[ry + 1][rx] == Path) {
            m_maze[ry][rx] = Path;
        }
    }
}