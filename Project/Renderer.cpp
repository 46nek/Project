#include "Renderer.h"
#include "Game.h"

Renderer::Renderer(GraphicsDevice* graphicsDevice) : m_graphicsDevice(graphicsDevice) {}
Renderer::~Renderer() {}

void Renderer::RenderDepthPass(const std::vector<std::unique_ptr<Model>>& models, LightManager* lightManager)
{
    ID3D11DeviceContext* deviceContext = m_graphicsDevice->GetDeviceContext();
    ShaderManager* shaderManager = m_graphicsDevice->GetShaderManager();

    // �`�����V���h�E�}�b�v�ɐݒ�
    m_graphicsDevice->GetShadowMapper()->SetRenderTarget(deviceContext);

    // �[�x���݂̂������o���V�F�[�_�[��ݒ�
    deviceContext->IASetInputLayout(shaderManager->GetInputLayout());
    deviceContext->VSSetShader(shaderManager->GetDepthVertexShader(), nullptr, 0);
    deviceContext->PSSetShader(nullptr, nullptr, 0); // �s�N�Z���V�F�[�_�[�͕s�v

    // �S�Ẵ��f����`��
    for (const auto& model : models) {
        if (model) {
            m_graphicsDevice->UpdateMatrixBuffer(
                model->GetWorldMatrix(),
                lightManager->GetLightViewMatrix(),
                lightManager->GetLightProjectionMatrix(),
                lightManager->GetLightViewMatrix(), // LightView
                lightManager->GetLightProjectionMatrix() // LightProjection
            );
            model->Render(deviceContext);
        }
    }
}

void Renderer::RenderMainPass(const std::vector<std::unique_ptr<Model>>& models, const Camera* camera, LightManager* lightManager)
{
    ID3D11DeviceContext* deviceContext = m_graphicsDevice->GetDeviceContext();
    ShaderManager* shaderManager = m_graphicsDevice->GetShaderManager();
    ShadowMapper* shadowMapper = m_graphicsDevice->GetShadowMapper();

    // �r���[�|�[�g����ʃT�C�Y�Ƀ��Z�b�g
    D3D11_VIEWPORT vp = {};
    vp.Width = static_cast<float>(Game::SCREEN_WIDTH);
    vp.Height = static_cast<float>(Game::SCREEN_HEIGHT);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    deviceContext->RSSetViewports(1, &vp);

    // ���X�^���C�U�[�X�e�[�g���f�t�H���g�ɖ߂�
    deviceContext->RSSetState(nullptr);

    // Z�o�b�t�@��L���ɂ���
    m_graphicsDevice->GetSwapChain()->TurnZBufferOn(deviceContext);

    // �ʏ��3D�V�[���p�̃V�F�[�_�[��ݒ�
    deviceContext->IASetInputLayout(shaderManager->GetInputLayout());
    deviceContext->VSSetShader(shaderManager->GetVertexShader(), nullptr, 0);
    deviceContext->PSSetShader(shaderManager->GetPixelShader(), nullptr, 0);

    // ���C�g�̏����V�F�[�_�[�ɓn��
    m_graphicsDevice->UpdateLightBuffer(lightManager->GetLightBuffer());

    // �e�N�X�`���T���v���[��ݒ�
    ID3D11SamplerState* samplerState = m_graphicsDevice->GetSamplerState();
    deviceContext->PSSetSamplers(0, 1, &samplerState);

    // �V���h�E�}�b�v���e�N�X�`���Ƃ��ăV�F�[�_�[�ɓn��
    ID3D11ShaderResourceView* shadowSrv = shadowMapper->GetShadowMapSRV();
    deviceContext->PSSetShaderResources(1, 1, &shadowSrv);
    ID3D11SamplerState* shadowSampler = shadowMapper->GetShadowSampleState();
    deviceContext->PSSetSamplers(1, 1, &shadowSampler);

    // �S�Ẵ��f����`��
    for (const auto& model : models) {
        if (model) {
            m_graphicsDevice->UpdateMatrixBuffer(
                model->GetWorldMatrix(),
                camera->GetViewMatrix(),
                DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PI / 4.0f, (float)Game::SCREEN_WIDTH / Game::SCREEN_HEIGHT, 0.1f, 1000.0f),
                lightManager->GetLightViewMatrix(),
                lightManager->GetLightProjectionMatrix()
            );
            model->Render(deviceContext);
        }
    }
}