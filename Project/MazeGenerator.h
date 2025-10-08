#pragma once
#include <vector>

class MazeGenerator
{
public:
    // ���H�̃Z���̎��
    enum CellType
    {
        Path, // ��
        Wall  // ��
    };

    MazeGenerator();
    ~MazeGenerator();

    // ���H�𐶐�����֐�
    void Generate(int width, int height);

    // �������ꂽ���H�̃f�[�^���擾����֐�
    const std::vector<std::vector<CellType>>& GetMazeData() const;

private:
    // �[���D��T���Ŗ��H���@��i�߂�ċA�֐�
    void CarvePath(int x, int y);

    int m_width;  // ���H�̕�
    int m_height; // ���H�̍���
    std::vector<std::vector<CellType>> m_maze; // ���H�̃f�[�^��ێ�����2�����z��
};