#pragma once
#include <DirectXMath.h>

/**
 * @brief �v���C���[�̎��_���Ǘ�����J�����N���X
 */
class Camera
{
public:
    Camera();
    ~Camera();

    /**
     * @brief �J�����̈ʒu��ݒ肵�܂��B
     * @param x X���W
     * @param y Y���W
     * @param z Z���W
     */
    void SetPosition(float x, float y, float z);

    /**
     * @brief �J�����̉�]�p�x��ݒ肵�܂��B
     * @param x X������̉�]�i�s�b�`�j
     * @param y Y������̉�]�i���[�j
     * @param z Z������̉�]�i���[���j
     */
    void SetRotation(float x, float y, float z);

    /**
     * @brief ���݂̃J�����̈ʒu���擾���܂��B
     * @return DirectX::XMFLOAT3�^�̈ʒu���W
     */
    DirectX::XMFLOAT3 GetPosition() const;

    /**
     * @brief ���݂̃J�����̉�]�p�x���擾���܂��B
     * @return DirectX::XMFLOAT3�^�̉�]�p�x
     */
    DirectX::XMFLOAT3 GetRotation() const;

    /**
     * @brief �J�����̏�Ԃ��X�V���A�r���[�s����Čv�Z���܂��B
     */
    void Update();

    /**
     * @brief �v�Z�ς݂̃r���[�s����擾���܂��B
     * @return DirectX::XMMATRIX�^�̃r���[�s��
     */
    DirectX::XMMATRIX GetViewMatrix() const;

    void MoveForward(float deltaTime);
    void MoveBackward(float deltaTime);
    void MoveLeft(float deltaTime);
    void MoveRight(float deltaTime);
    void Turn(int mouseX, int mouseY, float deltaTime);

private:
    // �ʒu
    float m_positionX, m_positionY, m_positionZ;
    // ��]
    float m_rotationX, m_rotationY, m_rotationZ;
    // �r���[�s��
    DirectX::XMMATRIX m_viewMatrix;
    // ���x
    float m_moveSpeed;
    float m_rotationSpeed;
};