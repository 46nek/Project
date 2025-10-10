#include "Renderer.h"
#include "../Core/Game.h" // スクリーンサイズ取得のため

Renderer::Renderer(GraphicsDevice* graphicsDevice) : m_graphicsDevice(graphicsDevice) {}
Renderer::~Renderer() {}

void Renderer::RenderScene(const std::vector<std::unique_ptr<Model>>& models, const Camera* camera, LightManager* lightManager)
{
    if (!m_graphicsDevice || models.empty() || !camera || !lightManager) return;

    RenderDepthPass(models, lightManager);
    RenderMainPass(models, camera, lightManager);
}

void Renderer::RenderDepthPass(const std::vector<std::unique_ptr<Model>>& models, LightManager* lightManager)
{
    m_graphicsDevice->GetShadowMapper()->SetRenderTarget(m_graphicsDevice->GetDeviceContext());

    auto deviceContext = m_graphicsDevice->GetDeviceContext();
    auto shaderManager = m_graphicsDevice->GetShaderManager();

    deviceContext->IASetInputLayout(shaderManager->GetInputLayout());
    deviceContext->VSSetShader(shaderManager->GetDepthVertexShader(), nullptr, 0);
    deviceContext->PSSetShader(nullptr, nullptr, 0); // Pixel shader is not needed

    // Update matrix buffer for depth pass
    m_graphicsDevice->UpdateMatrixBuffer(
        DirectX::XMMatrixIdentity(), // World matrix is set per-model
        lightManager->GetLightViewMatrix(),
        lightManager->GetLightProjectionMatrix(),
        lightManager->GetLightViewMatrix(),
        lightManager->GetLightProjectionMatrix()
    );

    DrawModels(models);
}

void Renderer::RenderMainPass(const std::vector<std::unique_ptr<Model>>& models, const Camera* camera, LightManager* lightManager)
{
    // Reset render target to the main swap chain
    ID3D11DeviceContext* deviceContext = m_graphicsDevice->GetDeviceContext();
    D3D11_VIEWPORT vp = {};
    vp.Width = Game::SCREEN_WIDTH;
    vp.Height = Game::SCREEN_HEIGHT;
    vp.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &vp);
    deviceContext->RSSetState(nullptr);

    m_graphicsDevice->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
    m_graphicsDevice->GetSwapChain()->TurnZBufferOn(deviceContext);

    auto shaderManager = m_graphicsDevice->GetShaderManager();
    deviceContext->IASetInputLayout(shaderManager->GetInputLayout());
    deviceContext->VSSetShader(shaderManager->GetVertexShader(), nullptr, 0);
    deviceContext->PSSetShader(shaderManager->GetPixelShader(), nullptr, 0);

    // Update light buffer
    m_graphicsDevice->UpdateLightBuffer(lightManager->GetLightBuffer());

    // Set shadow map resources
    auto shadowMapper = m_graphicsDevice->GetShadowMapper();
    ID3D11ShaderResourceView* shadowSrv = shadowMapper->GetShadowMapSRV();
    deviceContext->PSSetShaderResources(1, 1, &shadowSrv);
    ID3D11SamplerState* shadowSampler = shadowMapper->GetShadowSampleState();
    deviceContext->PSSetSamplers(1, 1, &shadowSampler);

    // Update matrix buffer for main pass
    m_graphicsDevice->UpdateMatrixBuffer(
        DirectX::XMMatrixIdentity(), // World matrix is set per-model
        camera->GetViewMatrix(),
        DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, (float)Game::SCREEN_WIDTH / Game::SCREEN_HEIGHT, 0.1f, 1000.0f),
        lightManager->GetLightViewMatrix(),
        lightManager->GetLightProjectionMatrix()
    );

    DrawModels(models);

    m_graphicsDevice->EndScene();
}

void Renderer::DrawModels(const std::vector<std::unique_ptr<Model>>& models)
{
    for (const auto& model : models)
    {
        if (model)
        {
            // ここで各モデルのワールド行列を更新する必要があります
            // 本来はRendererが直接ModelのTransformを触るのではなく、
            // 描画データとして受け取るのが望ましいですが、今回は簡略化します。
            // m_graphicsDevice->UpdateMatrixBuffer(...); // ワールド行列だけを更新するようなメソッドが別途必要
            model->Render(m_graphicsDevice->GetDeviceContext());
        }
    }
}