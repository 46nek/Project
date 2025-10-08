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
    // do-while���[�v���폜���A�S�[���𒼐ڐݒ肵�܂��B
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