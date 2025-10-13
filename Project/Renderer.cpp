// Renderer.cpp (���S�㏑��)

#include "Renderer.h"
#include "Game.h"

Renderer::Renderer(GraphicsDevice* graphicsDevice) : m_graphicsDevice(graphicsDevice) {}
Renderer::~Renderer() {}

void Renderer::RenderSceneToTexture(const std::vector<std::unique_ptr<Model>>& models, const Camera* camera, LightManager* lightManager)
{
    if (!m_graphicsDevice || !camera || !lightManager) return;

    RenderTarget* renderTarget = m_graphicsDevice->GetRenderTarget();
    renderTarget->SetRenderTarget(m_graphicsDevice->GetDeviceContext());
    renderTarget->ClearRenderTarget(m_graphicsDevice->GetDeviceContext(), 0.1f, 0.2f, 0.4f, 1.0f);

    RenderDepthPass(models, lightManager);

    renderTarget->SetRenderTarget(m_graphicsDevice->GetDeviceContext());

    RenderMainPass(models, camera, lightManager);
}

void Renderer::RenderFinalPass(const Camera* camera)
{
    ID3D11DeviceContext* deviceContext = m_graphicsDevice->GetDeviceContext();
    ShaderManager* shaderManager = m_graphicsDevice->GetShaderManager();

    m_graphicsDevice->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
    m_graphicsDevice->GetSwapChain()->TurnZBufferOff(deviceContext);

    deviceContext->IASetInputLayout(shaderManager->GetInputLayout());

    // ������ �����Ń|�X�g�v���Z�X��p�̒��_�V�F�[�_�[���g�p ������
    deviceContext->VSSetShader(shaderManager->GetPostProcessVertexShader(), nullptr, 0);
    // ������ �ύX�_ ������

    // ������ �����Ń��[�V�����u���[�̃V�F�[�_�[���g�p ������
    deviceContext->PSSetShader(shaderManager->GetMotionBlurPixelShader(), nullptr, 0);
    // ������ �ύX�_ ������

    // ���[�V�����u���[�p�̒萔�o�b�t�@���X�V
    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PI / 4.0f, (float)Game::SCREEN_WIDTH / Game::SCREEN_HEIGHT, 0.1f, 1000.0f);
    DirectX::XMMATRIX prevViewProj = camera->GetPreviousViewMatrix() * projectionMatrix;
    DirectX::XMMATRIX currentViewProj = camera->GetViewMatrix() * projectionMatrix;
    DirectX::XMMATRIX currentViewProjInv = DirectX::XMMatrixInverse(nullptr, currentViewProj);
    m_graphicsDevice->UpdateMotionBlurBuffer(prevViewProj, currentViewProjInv, 1.0f);

    // 3D���f���p�̍s��o�b�t�@�͎g��Ȃ��̂ŁA�P�ʍs���ݒ肵�Ă����i���S�̂��߁j
    DirectX::XMMATRIX identity = DirectX::XMMatrixIdentity();
    m_graphicsDevice->UpdateMatrixBuffer(identity, identity, identity, identity, identity);

    // �V�[����`�悵���e�N�X�`���Ɛ[�x�e�N�X�`�����V�F�[�_�[�ɓn��
    ID3D11ShaderResourceView* sceneTexture = m_graphicsDevice->GetRenderTarget()->GetShaderResourceView();
    deviceContext->PSSetShaderResources(0, 1, &sceneTexture);
    ID3D11ShaderResourceView* depthTexture = m_graphicsDevice->GetRenderTarget()->GetDepthShaderResourceView();
    deviceContext->PSSetShaderResources(1, 1, &depthTexture);

    ID3D11SamplerState* samplerState = m_graphicsDevice->GetSamplerState();
    deviceContext->PSSetSamplers(0, 1, &samplerState);

    m_graphicsDevice->GetOrthoWindow()->Render(deviceContext);

    m_graphicsDevice->GetSwapChain()->TurnZBufferOn(deviceContext);

    ID3D11ShaderResourceView* nullSRVs[2] = { nullptr, nullptr };
    deviceContext->PSSetShaderResources(0, 2, nullSRVs);
}

void Renderer::RenderDepthPass(const std::vector<std::unique_ptr<Model>>& models, LightManager* lightManager)
{
    // (���̊֐��̒��g�͕ύX����܂���)
    ID3D11DeviceContext* deviceContext = m_graphicsDevice->GetDeviceContext();
    ShaderManager* shaderManager = m_graphicsDevice->GetShaderManager();

    m_graphicsDevice->GetShadowMapper()->SetRenderTarget(deviceContext);

    deviceContext->IASetInputLayout(shaderManager->GetInputLayout());
    deviceContext->VSSetShader(shaderManager->GetDepthVertexShader(), nullptr, 0);
    deviceContext->PSSetShader(nullptr, nullptr, 0);

    for (const auto& model : models) {
        if (model) {
            m_graphicsDevice->UpdateMatrixBuffer(
                model->GetWorldMatrix(),
                lightManager->GetLightViewMatrix(),
                lightManager->GetLightProjectionMatrix(),
                lightManager->GetLightViewMatrix(),
                lightManager->GetLightProjectionMatrix()
            );
            model->Render(deviceContext);
        }
    }
}

void Renderer::RenderMainPass(const std::vector<std::unique_ptr<Model>>& models, const Camera* camera, LightManager* lightManager)
{
    // (���̊֐��̒��g�͕ύX����܂���)
    ID3D11DeviceContext* deviceContext = m_graphicsDevice->GetDeviceContext();
    ShaderManager* shaderManager = m_graphicsDevice->GetShaderManager();
    ShadowMapper* shadowMapper = m_graphicsDevice->GetShadowMapper();

    D3D11_VIEWPORT vp = {};
    vp.Width = Game::SCREEN_WIDTH;
    vp.Height = Game::SCREEN_HEIGHT;
    vp.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &vp);
    deviceContext->RSSetState(nullptr);

    m_graphicsDevice->GetSwapChain()->TurnZBufferOn(deviceContext);

    deviceContext->IASetInputLayout(shaderManager->GetInputLayout());
    deviceContext->VSSetShader(shaderManager->GetVertexShader(), nullptr, 0);
    deviceContext->PSSetShader(shaderManager->GetPixelShader(), nullptr, 0);

    m_graphicsDevice->UpdateLightBuffer(lightManager->GetLightBuffer());

    ID3D11SamplerState* samplerState = m_graphicsDevice->GetSamplerState();
    deviceContext->PSSetSamplers(0, 1, &samplerState);

    ID3D11ShaderResourceView* shadowSrv = shadowMapper->GetShadowMapSRV();
    deviceContext->PSSetShaderResources(1, 1, &shadowSrv);
    ID3D11SamplerState* shadowSampler = shadowMapper->GetShadowSampleState();
    deviceContext->PSSetSamplers(1, 1, &shadowSampler);

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