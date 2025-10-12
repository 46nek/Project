#pragma once
#include "GraphicsDevice.h"
#include <memory>
#include <wrl/client.h>

class PostProcess
{
public:
    PostProcess();
    ~PostProcess();

    bool Initialize(GraphicsDevice* graphicsDevice, int screenWidth, int screenHeight);
    void Shutdown();

    // �V�[���̕`����J�n���邽�߂ɁA�����_�[�^�[�Q�b�g��ݒ肵�܂�
    void SetRenderTarget(ID3D11DeviceContext* deviceContext);
    // �|�X�g�v���Z�X�G�t�F�N�g��K�p���A�ŏI�I�Ȍ��ʂ��o�b�N�o�b�t�@�ɕ`�悵�܂�
    void Apply(ID3D11DeviceContext* deviceContext);

private:
    // �t���X�N���[���̃|���S����`�悷��w���p�[�֐�
    void RenderFullscreenQuad(ID3D11DeviceContext* deviceContext);

    GraphicsDevice* m_graphicsDevice;

    // �V�[���S�̂������_�����O���邽�߂̃����_�[�^�[�Q�b�g
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_sceneTexture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_sceneRTV;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_sceneSRV;

    // �u���[�������p�̈ꎞ�I�ȃ����_�[�^�[�Q�b�g
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_bloomTexture1;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_bloomRTV1;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_bloomSRV1;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_bloomTexture2;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_bloomRTV2;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_bloomSRV2;

    // �t���X�N���[���N�A�b�h�p�̒��_�E�C���f�b�N�X�o�b�t�@
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;

    // �K�E�V�A���u���[�p�̒萔�o�b�t�@
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_blurBuffer;

    // �ڂ��������̒萔�o�b�t�@�\����
    struct BlurBufferType
    {
        DirectX::XMFLOAT2 textureSize;
        DirectX::XMFLOAT2 blurDirection;
    };
};