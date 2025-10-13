#pragma once
#include <DirectXMath.h>
#include <vector>
#include "Light.h"
#include "MazeGenerator.h" // MazeGenerator���C���N���[�h

/**
 * @struct LightBufferType
 * @brief �V�F�[�_�[�ɓn�����C�g���S�̂��i�[����\����
 */
struct LightBufferType
{
    Light               Lights[16];
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

    // Initialize�̈�����ύX
    void Initialize(const std::vector<std::vector<MazeGenerator::CellType>>& mazeData, float pathWidth, float wallHeight);
    // Update�̈�����ύX
    void Update(float deltaTime, const DirectX::XMFLOAT3& cameraPosition);

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