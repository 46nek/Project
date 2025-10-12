#pragma once
#include <vector>
#include <memory>
#include "GraphicsDevice.h"
#include "Model.h"
#include "LightManager.h"
#include "Camera.h"

/**
 * @brief 3D�V�[���̕`�揈���𓝊�����N���X
 */
class Renderer
{
public:
    Renderer(GraphicsDevice* graphicsDevice);
    ~Renderer();
    /**
        * @brief �V���h�E�}�b�v�����p�X
        */
    void RenderDepthPass(const std::vector<std::unique_ptr<Model>>& models, LightManager* lightManager);

    /**
     * @brief �ʏ�̕`��p�X
     */
    void RenderMainPass(const std::vector<std::unique_ptr<Model>>& models, const Camera* camera, LightManager* lightManager);
    // --- ������ �����܂ŏC�� ������ ---

private:
    GraphicsDevice* m_graphicsDevice;
};