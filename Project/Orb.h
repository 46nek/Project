#pragma once
#include "Model.h"
#include "Player.h"
#include "LightManager.h"

/**
 * @brief ����\�ȃI�[�u���Ǘ�����N���X
 */
class Orb
{
public:
    Orb();
    ~Orb();

    /**
     * @brief �I�[�u�����������܂�
     * @param device Direct3D�f�o�C�X
     * @param position �I�[�u�̏����ʒu
     * @param lightIndex LightManager�ŊǗ�����郉�C�g�̃C���f�b�N�X
     * @return �������ɐ��������ꍇ��true
     */
    bool Initialize(ID3D11Device* device, const DirectX::XMFLOAT3& position, int lightIndex);

    /**
     * @brief ���\�[�X��������܂�
     */
    void Shutdown();

    /**
     * @brief �I�[�u�̏�Ԃ��X�V���܂��i���V�A�j���[�V�����A�v���C���[�Ƃ̓����蔻��j
     * @param deltaTime �O�t���[������̌o�ߎ���
     * @param player �v���C���[�I�u�W�F�N�g
     * @param lightManager ���C�g�Ǘ��I�u�W�F�N�g
     */
    void Update(float deltaTime, Player* player, LightManager* lightManager);

    /**
     * @brief �`��p�̃��f�����擾���܂��i������̏ꍇ�̂݁j
     * @return ���f���ւ̃|�C���^�B����ς݂̏ꍇ��nullptr
     */
    Model* GetModel();

    /**
     * @brief �I�[�u��������ꂽ���ǂ������擾���܂�
     * @return ����ς݂̏ꍇ��true
     */
    bool IsCollected() const;

private:
    std::unique_ptr<Model> m_model;
    DirectX::XMFLOAT3 m_position;
    bool m_isCollected;
    int m_lightIndex;

    // ���V�A�j���[�V�����p�̃^�C�}�[
    float m_animationTimer;
};