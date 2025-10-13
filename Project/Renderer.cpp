// Renderer.cpp (完全上書き)

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

    // ▼▼▼ ここでポストプロセス専用の頂点シェーダーを使用 ▼▼▼
    deviceContext->VSSetShader(shaderManager->GetPostProcessVertexShader(), nullptr, 0);
    // ▲▲▲ 変更点 ▲▲▲

    // ▼▼▼ ここでモーションブラーのシェーダーを使用 ▼▼▼
    deviceContext->PSSetShader(shaderManager->GetMotionBlurPixelShader(), nullptr, 0);
    // ▲▲▲ 変更点 ▲▲▲

    // モーションブラー用の定数バッファを更新
    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PI / 4.0f, (float)Game::SCREEN_WIDTH / Game::SCREEN_HEIGHT, 0.1f, 1000.0f);
    DirectX::XMMATRIX prevViewProj = camera->GetPreviousViewMatrix() * projectionMatrix;
    DirectX::XMMATRIX currentViewProj = camera->GetViewMatrix() * projectionMatrix;
    DirectX::XMMATRIX currentViewProjInv = DirectX::XMMatrixInverse(nullptr, currentViewProj);
    m_graphicsDevice->UpdateMotionBlurBuffer(prevViewProj, currentViewProjInv, 1.0f);

    // 3Dモデル用の行列バッファは使わないので、単位行列を設定しておく（安全のため）
    DirectX::XMMATRIX identity = DirectX::XMMatrixIdentity();
    m_graphicsDevice->UpdateMatrixBuffer(identity, identity, identity, identity, identity);

    // シーンを描画したテクスチャと深度テクスチャをシェーダーに渡す
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
    // (この関数の中身は変更ありません)
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
    // (この関数の中身は変更ありません)
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