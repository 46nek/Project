#pragma once
#include <d3d11.h>
#include <string>
#include <vector>
#include <memory>
#include "Model.h"
#include "MazeGenerator.h"
#include "MeshGenerator.h"

/**
 * @class AssetLoader
 * @brief 3D���f����e�N�X�`���Ȃǂ̃A�Z�b�g���t�@�C������ǂݍ��ސÓI�N���X
 */
class AssetLoader
{
public:
    /**
     * @brief Assimp���C�u�������g�p���ă��f���t�@�C����ǂݍ���
     * @return �ǂݍ��݂ɐ��������ꍇ��Model�̃��j�[�N�|�C���^
     */
    static std::unique_ptr<Model> LoadModelFromFile(ID3D11Device* device, const std::string& filename);

    /**
     * @brief ���H�f�[�^����3D���f���𐶐�����
     * @return �����ɐ��������ꍇ��Model�̃��j�[�N�|�C���^
     */
    static std::unique_ptr<Model> CreateMazeModel(ID3D11Device* device, const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth, float wallHeight, MeshGenerator::MeshType type);

    /**
     * @brief �e�N�X�`���t�@�C����ǂݍ���
     * @return �ǂݍ��݂ɐ��������ꍇ��Texture�̃��j�[�N�|�C���^
     */
    static std::unique_ptr<Texture> LoadTexture(ID3D11Device* device, const wchar_t* filename);
};