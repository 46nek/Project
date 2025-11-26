#include "Renderer.h"
#include "Game.h"
#include "Stage.h"
#include <algorithm>
#include <cmath>

Renderer::Renderer(GraphicsDevice* graphicsDevice) : m_graphicsDevice(graphicsDevice) {
	m_frustum = std::make_unique<Frustum>();
}

Renderer::~Renderer() {
}

void Renderer::RenderSceneToTexture(
	const std::vector<Model*>& stageModels,
	const std::vector<Model*>& dynamicModels,
	const Camera* camera,
	LightManager* lightManager,
	const std::vector<std::vector<MazeGenerator::CellType>>& mazeData,
	float pathWidth) {
	if (!m_graphicsDevice || !camera || !lightManager) return;

	RenderTarget* renderTarget = m_graphicsDevice->GetRenderTarget();
	renderTarget->SetRenderTarget(m_graphicsDevice->GetDeviceContext());
	renderTarget->ClearRenderTarget(m_graphicsDevice->GetDeviceContext(), 0.1f, 0.2f, 0.4f, 1.0f);

	RenderDepthPass(stageModels, dynamicModels, lightManager);

	renderTarget->SetRenderTarget(m_graphicsDevice->GetDeviceContext());

	RenderMainPass(stageModels, dynamicModels, camera, lightManager, mazeData, pathWidth);
}

void Renderer::RenderFinalPass(const Camera* camera, float vignetteIntensity) {
	ID3D11DeviceContext* deviceContext = m_graphicsDevice->GetDeviceContext();
	ShaderManager* shaderManager = m_graphicsDevice->GetShaderManager();

	// ポストプロセス用の定数バッファを更新
	PostProcessBufferType postProcessBuffer;
	postProcessBuffer.VignetteIntensity = vignetteIntensity;
	m_graphicsDevice->UpdatePostProcessBuffer(postProcessBuffer);

	m_graphicsDevice->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
	m_graphicsDevice->GetSwapChain()->TurnZBufferOff(deviceContext);

	deviceContext->IASetInputLayout(shaderManager->GetInputLayout());
	deviceContext->VSSetShader(shaderManager->GetPostProcessVertexShader(), nullptr, 0);
	deviceContext->PSSetShader(shaderManager->GetMotionBlurPixelShader(), nullptr, 0);

	DirectX::XMMATRIX currentView = camera->GetViewMatrix();
	DirectX::XMMATRIX previousView = camera->GetPreviousViewMatrix();

	float blurAmount = 1.0f;
	if (memcmp(&currentView, &previousView, sizeof(DirectX::XMMATRIX)) == 0) {
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

void Renderer::RenderDepthPass(
	const std::vector<Model*>& stageModels,
	const std::vector<Model*>& dynamicModels,
	LightManager* lightManager) {
	ID3D11DeviceContext* deviceContext = m_graphicsDevice->GetDeviceContext();
	ShaderManager* shaderManager = m_graphicsDevice->GetShaderManager();

	m_graphicsDevice->GetShadowMapper()->SetRenderTarget(deviceContext);

	deviceContext->IASetInputLayout(shaderManager->GetInputLayout());
	deviceContext->VSSetShader(shaderManager->GetDepthVertexShader(), nullptr, 0);
	deviceContext->PSSetShader(nullptr, nullptr, 0);

	// ラムダ式で描画処理を共通化
	auto renderDepth = [&](Model* model) {
		if (!model) return;

		if (!m_frustum->CheckSphere(model->GetBoundingSphereCenter(), model->GetBoundingSphereRadius())) {
			return;
		}

		m_graphicsDevice->UpdateMatrixBuffer(
			model->GetWorldMatrix(),
			lightManager->GetLightViewMatrix(),
			lightManager->GetLightProjectionMatrix(),
			lightManager->GetLightViewMatrix(),
			lightManager->GetLightProjectionMatrix()
		);
		model->Render(deviceContext);
		};

	for (Model* model : stageModels) {
		renderDepth(model);
	}
	for (Model* model : dynamicModels) {
		renderDepth(model);
	}
}

void Renderer::RenderMainPass(
	const std::vector<Model*>& stageModels,
	const std::vector<Model*>& dynamicModels,
	const Camera* camera,
	LightManager* lightManager,
	const std::vector<std::vector<MazeGenerator::CellType>>& mazeData,
	float pathWidth) {
	ID3D11DeviceContext* deviceContext = m_graphicsDevice->GetDeviceContext();
	ShaderManager* shaderManager = m_graphicsDevice->GetShaderManager();
	ShadowMapper* shadowMapper = m_graphicsDevice->GetShadowMapper();

	DirectX::XMMATRIX viewMatrix = camera->GetViewMatrix();
	DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PI / 4.0f, (float)Game::SCREEN_WIDTH / Game::SCREEN_HEIGHT, 0.1f, 1000.0f);
	m_frustum->ConstructFrustum(viewMatrix, projectionMatrix);

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

	struct CurrentMaterialState {
		DirectX::XMFLOAT4 EmissiveColor;
		int UseTexture;
		int UseNormalMap;
		bool IsValid = false;
	} currentState;

	auto renderModel = [&](Model* model) {
		if (!model) return;

		bool needUpdate = !currentState.IsValid;
		if (currentState.IsValid) {
			auto ec = model->GetEmissiveColor();
			if (ec.x != currentState.EmissiveColor.x ||
				ec.y != currentState.EmissiveColor.y ||
				ec.z != currentState.EmissiveColor.z ||
				model->GetUseTexture() != currentState.UseTexture ||
				(model->HasNormalMap() && model->GetUseNormalMap()) != currentState.UseNormalMap) {
				needUpdate = true;
			}
		}

		if (needUpdate) {
			MaterialBufferType materialBuffer;
			materialBuffer.EmissiveColor = model->GetEmissiveColor();
			materialBuffer.UseTexture = model->GetUseTexture();
			materialBuffer.UseNormalMap = model->HasNormalMap() && model->GetUseNormalMap();
			m_graphicsDevice->UpdateMaterialBuffer(materialBuffer);

			currentState.EmissiveColor = materialBuffer.EmissiveColor;
			currentState.UseTexture = materialBuffer.UseTexture;
			currentState.UseNormalMap = materialBuffer.UseNormalMap;
			currentState.IsValid = true;
		}

		m_graphicsDevice->UpdateMatrixBuffer(
			model->GetWorldMatrix(),
			viewMatrix,
			projectionMatrix,
			lightManager->GetLightViewMatrix(),
			lightManager->GetLightProjectionMatrix()
		);
		model->Render(deviceContext);
		};

	// 1. ステージモデル
	for (Model* model : stageModels) {
		if (!m_frustum->CheckSphere(model->GetBoundingSphereCenter(), model->GetBoundingSphereRadius())) {
			continue;
		}
		renderModel(model);
	}

	// 2. 動的モデル
	for (Model* model : dynamicModels) {
		DirectX::XMFLOAT3 modelPos = model->GetBoundingSphereCenter();
		if (!m_frustum->CheckSphere(modelPos, model->GetBoundingSphereRadius())) {
			continue;
		}
		renderModel(model);
	}

	deviceContext->OMSetBlendState(m_graphicsDevice->GetDefaultBlendState(), blendFactor, 0xffffffff);
}