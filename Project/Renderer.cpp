#include "Renderer.h"
#include "Game.h"

Renderer::Renderer(GraphicsDevice* graphicsDevice) : m_graphicsDevice(graphicsDevice) {}
Renderer::~Renderer() {}

void Renderer::RenderScene(const std::vector<std::unique_ptr<Model>>& models, const Camera* camera, LightManager* lightManager)
{
    if (!m_graphicsDevice || !camera || !lightManager) return;

    RenderDepthPass(models, lightManager);
    RenderMainPass(models, camera, lightManager);
}

void Renderer::RenderDepthPass(const std::vector<std::unique_ptr<Model>>& models, LightManager* lightManager)
{
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
    ID3D11DeviceContext* deviceContext = m_graphicsDevice->GetDeviceContext();
    ShaderManager* shaderManager = m_graphicsDevice->GetShaderManager();
    ShadowMapper* shadowMapper = m_graphicsDevice->GetShadowMapper();

    // Reset render target
    D3D11_VIEWPORT vp = {};
    vp.Width = Game::SCREEN_WIDTH;
    vp.Height = Game::SCREEN_HEIGHT;
    vp.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &vp);
    deviceContext->RSSetState(nullptr);

    m_graphicsDevice->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
    m_graphicsDevice->GetSwapChain()->TurnZBufferOn(deviceContext);

    // Set shaders
    deviceContext->IASetInputLayout(shaderManager->GetInputLayout());
    deviceContext->VSSetShader(shaderManager->GetVertexShader(), nullptr, 0);
    deviceContext->PSSetShader(shaderManager->GetPixelShader(), nullptr, 0);

    // Update light buffer
    m_graphicsDevice->UpdateLightBuffer(lightManager->GetLightBuffer());

    // Set texture sampler
    ID3D11SamplerState* samplerState = m_graphicsDevice->GetSamplerState();
    deviceContext->PSSetSamplers(0, 1, &samplerState);

    // Set shadow map resources
    ID3D11ShaderResourceView* shadowSrv = shadowMapper->GetShadowMapSRV();
    deviceContext->PSSetShaderResources(1, 1, &shadowSrv);
    ID3D11SamplerState* shadowSampler = shadowMapper->GetShadowSampleState();
    deviceContext->PSSetSamplers(1, 1, &shadowSampler);

    // Draw each model
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

    m_graphicsDevice->EndScene();
}