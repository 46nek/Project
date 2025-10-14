// LightManager.h

#pragma once
#include <DirectXMath.h>
#include <vector>
#include "Light.h"
#include "MazeGenerator.h"

/**
 * @struct LightBufferType
 * @brief �V�F�[�_�[�ɓn�����C�g���S�̂��i�[����\����
 */
struct LightBufferType
{
    Light               Lights[32]; // ���C�g�̍ő吔��16����32�ɑ��₵�܂�
    int                 NumLights;
    DirectX::XMFLOAT3   CameraPosition;
};

/**
 * @class LightManager
 * @brief �V�[�����̑S�Ẵ��C�g���Ǘ����A�V�F�[�_�[�p�̃o�b�t�@���X�V
 */
class LightManager
{
public:
    LightManager();
    ~LightManager();

    void Initialize(const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth, float wallHeight);
    void Update(float deltaTime, const DirectX::XMFLOAT3& cameraPosition);

    /**
     * @brief �V�����|�C���g���C�g��ǉ����܂�
     * @return �ǉ����ꂽ���C�g�̃C���f�b�N�X�B����ɒB���Ă���ꍇ��-1
     */
    int AddPointLight(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT4& color, float range, float intensity);

    /**
     * @brief �w�肳�ꂽ�C���f�b�N�X�̃��C�g�̗L��/������؂�ւ��܂�
     * @param index ���C�g�̃C���f�b�N�X
     * @param enabled �L���ɂ���ꍇ��true
     */
    void SetLightEnabled(int index, bool enabled);


    // �Q�b�^�[
    const LightBufferType& GetLightBuffer() const { return m_lightBuffer; }
    DirectX::XMMATRIX GetLightViewMatrix() const { return m_lightViewMatrix; }
    DirectX::XMMATRIX GetLightProjectionMatrix() const { return m_lightProjectionMatrix; }

private:
    std::vector<Light> m_lights;
    LightBufferType m_lightBuffer;

    // �V���h�E�}�b�s���O�p
    DirectX::XMMATRIX m_lightViewMatrix;
    DirectX::XMMATRIX m_lightProjectionMatrix;
};