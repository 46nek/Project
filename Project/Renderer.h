#pragma once
#include <vector>
#include <memory>
#include "GraphicsDevice.h"
#include "Model.h"
#include "LightManager.h"
#include "Camera.h"
#include "PostProcess.h"

/**
 * @brief 3D�V�[���̕`�揈���𓝊�����N���X
 */
class Renderer
{
public:
    Renderer(GraphicsDevice* graphicsDevice);
    ~Renderer();

    /**
     * @brief �V�[���S�̂�`��
     * @param models �`�悷�郂�f���̃��X�g
     * @param camera �J����
     * @param lightManager ���C�g�}�l�[�W���[
     */
    void RenderScene(const std::vector<std::unique_ptr<Model>>& models, const Camera* camera, LightManager* lightManager);

private:
    // �V���h�E�}�b�v�����p�X
    void RenderDepthPass(const std::vector<std::unique_ptr<Model>>& models, LightManager* lightManager);
    // �ʏ�̕`��p�X
    void RenderMainPass(const std::vector<std::unique_ptr<Model>>& models, const Camera* camera, LightManager* lightManager);

    GraphicsDevice* m_graphicsDevice;
};