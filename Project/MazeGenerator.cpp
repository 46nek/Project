#include "MazeGenerator.h"
#include <vector>

MazeGenerator::MazeGenerator() : m_width(0), m_height(0), m_startX(0), m_startY(0)
{
}

MazeGenerator::~MazeGenerator()
{
}

// �w�肳�ꂽ�͈͂𓹂ɂ���i������ʘH�����j�֐�
void MazeGenerator::CreateRoom(int startX, int startY, int width, int height)
{
    // �}�b�v�͈̔͊O�ɂȂ�Ȃ��悤�Ƀ`�F�b�N
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
    // ���ƍ�����21x21�ɌŒ�
    m_width = 21;
    m_height = 21;

    // 1. �S�ẴZ����ǂŏ�����
    m_maze.assign(m_height, std::vector<CellType>(m_width, Wall));

    // 2. 3x3�̕�����z�u
    CreateRoom(1, 1, 3, 3);   // ����
    CreateRoom(17, 1, 3, 3);  // �E��
    CreateRoom(1, 17, 3, 3);  // ����
    CreateRoom(17, 17, 3, 3); // �E��
    CreateRoom(9, 9, 3, 3);   // ����

    // 3. �O���̒ʘH���쐬
    CreateRoom(4, 2, 13, 1);  // ��
    CreateRoom(4, 18, 13, 1); // ��
    CreateRoom(2, 4, 1, 13);  // ��
    CreateRoom(18, 4, 1, 13); // �E

    // 4. �����̕����ƊO�����Ȃ��ʘH���쐬
    CreateRoom(10, 3, 1, 6);  // �ォ�璆����
    CreateRoom(10, 12, 1, 6); // �������牺��
    CreateRoom(3, 10, 6, 1);  // �����璆����
    CreateRoom(12, 10, 6, 1); // ��������E��

    // 5. �X�^�[�g�n�_�𒆉��̕����ɐݒ�
    m_startX = 10;
    m_startY = 10;
    // �S�[���͐ݒ肹���A�O�ւ̏o�����Ȃ���
}

const std::vector<std::vector<MazeGenerator::CellType>>& MazeGenerator::GetMazeData() const
{
    return m_maze;
}

std::pair<int, int> MazeGenerator::GetStartPosition() const
{
    return { m_startX, m_startY };
}

// �ȉ��̊֐��͎g�p���Ȃ��̂ŋ�ɂ��܂�
void MazeGenerator::CarvePath(int x, int y) {}
void MazeGenerator::RemoveDeadEnds() {}
void MazeGenerator::CreateMoreLoops(int count) {}