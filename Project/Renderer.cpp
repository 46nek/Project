#include "Renderer.h"
#include "Game.h"

Renderer::Renderer(GraphicsDevice* graphicsDevice) : m_graphicsDevice(graphicsDevice)
{
	// --- ��������ǉ� ---
	m_frustum = std::make_unique<Frustum>();
	// --- �ǉ������܂� ---
}
Renderer::~Renderer() {}

void Renderer::RenderSceneToTexture(const std::vector<Model*>& models, const Camera* camera, LightManager* lightManager)
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
	deviceContext->VSSetShader(shaderManager->GetPostProcessVertexShader(), nullptr, 0);
	deviceContext->PSSetShader(shaderManager->GetMotionBlurPixelShader(), nullptr, 0);

	DirectX::XMMATRIX currentView = camera->GetViewMatrix();
	DirectX::XMMATRIX previousView = camera->GetPreviousViewMatrix();

	float blurAmount = 1.0f;
	if (memcmp(&currentView, &previousView, sizeof(DirectX::XMMATRIX)) == 0)
	{
		blurAmount = 0.0f;
	}

	DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PI / 4.0f, (float)Game::SCREEN_WIDTH / Game::SCREEN_HEIGHT, 0.1f, 1000.0f);
	DirectX::XMMATRIX prevViewProj = previousView * projectionMatrix;
	DirectX::XMMATRIX currentViewProj = currentView * projectionMatrix;
	DirectX::XMMATRIX currentViewProjInv = DirectX::XMMatrixInverse(nullptr, currentViewProj);
	m_graphicsDevice->UpdateMotionBlurBuffer(prevViewProj, currentViewProjInv, blurAmount);

	DirectX::XMMATRIX identity = DirectX::XMMatrixIdentity();
	m_graphicsDevice->UpdateMatrixBuffer(identity, identity, identity, identity, identity);

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

void Renderer::RenderDepthPass(const std::vector<Model*>& models, LightManager* lightManager)
{
	ID3D11DeviceContext* deviceContext = m_graphicsDevice->GetDeviceContext();
	ShaderManager* shaderManager = m_graphicsDevice->GetShaderManager();

	m_graphicsDevice->GetShadowMapper()->SetRenderTarget(deviceContext);

	deviceContext->IASetInputLayout(shaderManager->GetInputLayout());
	deviceContext->VSSetShader(shaderManager->GetDepthVertexShader(), nullptr, 0);
	deviceContext->PSSetShader(nullptr, nullptr, 0);

	for (Model* model : models) {
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

void Renderer::RenderMainPass(const std::vector<Model*>& models, const Camera* camera, LightManager* lightManager)
{
	ID3D11DeviceContext* deviceContext = m_graphicsDevice->GetDeviceContext();
	ShaderManager* shaderManager = m_graphicsDevice->GetShaderManager();
	ShadowMapper* shadowMapper = m_graphicsDevice->GetShadowMapper();

	// --- ��������ύX ---
	DirectX::XMMATRIX viewMatrix = camera->GetViewMatrix();
	DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PI / 4.0f, (float)Game::SCREEN_WIDTH / Game::SCREEN_HEIGHT, 0.1f, 1000.0f);
	m_frustum->ConstructFrustum(viewMatrix, projectionMatrix);
	// --- �ύX�����܂� ---

	D3D11_VIEWPORT vp = {};
	vp.Width = Game::SCREEN_WIDTH;
	vp.Height = Game::SCREEN_HEIGHT;
	vp.MaxDepth = 1.0f;
	deviceContext->RSSetViewports(1, &vp);
	deviceContext->RSSetState(nullptr);

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	deviceContext->OMSetBlendState(m_graphicsDevice->GetAlphaBlendState(), blendFactor, 0xffffffff);

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

	for (Model* model : models) {
		if (model) {
			// --- ��������ύX ---
			if (!m_frustum->CheckSphere(model->GetBoundingSphereCenter(), model->GetBoundingSphereRadius()))
			{
				continue; // ������̊O�ɂ��郂�f���͕`�悵�Ȃ�
			}
			// --- �ύX�����܂� ---

			MaterialBufferType materialBuffer;
			materialBuffer.EmissiveColor = model->GetEmissiveColor();
			materialBuffer.UseTexture = model->GetUseTexture();
			materialBuffer.UseNormalMap = model->HasNormalMap() && model->GetUseNormalMap();
			m_graphicsDevice->UpdateMaterialBuffer(materialBuffer);

			m_graphicsDevice->UpdateMatrixBuffer(
				model->GetWorldMatrix(),
				viewMatrix, // �X�V���ꂽviewMatrix���g�p
				projectionMatrix, // �X�V���ꂽprojectionMatrix���g�p
				lightManager->GetLightViewMatrix(),
				lightManager->GetLightProjectionMatrix()
			);
			model->Render(deviceContext);
		}
	}
	deviceContext->OMSetBlendState(m_graphicsDevice->GetDefaultBlendState(), blendFactor, 0xffffffff);
}