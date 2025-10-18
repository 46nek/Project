#pragma once
#include <vector>
#include <memory>
#include "GraphicsDevice.h"
#include "Model.h"
#include "LightManager.h"
#include "Camera.h"
#include "Frustum.h" // <--- �ǉ�

/**
 * @brief 3D�V�[���̕`�揈���𓝊�����N���X
 */
class Renderer
{
public:
	Renderer(GraphicsDevice* graphicsDevice);
	~Renderer();

	// --- �V�����`��t���[ ---
	// 1. �V�[�����e�N�X�`���Ƀ����_�����O����
	void RenderSceneToTexture(const std::vector<Model*>& models, const Camera* camera, LightManager* lightManager);
	// 2. �����_�����O���ꂽ�e�N�X�`���Ƀ|�X�g�v���Z�X��K�p���A��ʂɕ`�悷��
	void RenderFinalPass(const Camera* camera, float vignetteIntensity);
private:
	// �V���h�E�}�b�v�����p�X
	void RenderDepthPass(const std::vector<Model*>& models, LightManager* lightManager);
	// �ʏ�̕`��p�X
	void RenderMainPass(const std::vector<Model*>& models, const Camera* camera, LightManager* lightManager);

	GraphicsDevice* m_graphicsDevice;
	// --- ��������ǉ� ---
	std::unique_ptr<Frustum> m_frustum;
	// --- �ǉ������܂� ---
};